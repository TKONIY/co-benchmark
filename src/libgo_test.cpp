#include "benchmark.h"
#include <assert.h>
#include <chrono>
#include <fmt/core.h>
#include <iostream>
#include <libgo/coroutine.h>

static void libgo_create_join_test(int coroutine_n) {
  auto create_before = clk::now();

  co_chan<int> ch;
  for (int i = 0; i < coroutine_n; ++i) {
    go[ch] {
      Utils::f_null(nullptr);
      ch << 1;
    };
  }

  auto create_after = clk::now();
  auto create_duration = create_after - create_before;
  auto create_us = std::chrono::duration_cast<us>(create_duration).count();

  fmt::print("create {} threads, cost {} us\n", coroutine_n, create_us);

  // ignore new schedule thread's overhead
  std::thread([]() { co_sched.Start(std::thread::hardware_concurrency()); }).detach();

  auto ch_before = clk::now();

  int signal;
  for (int i = 0; i < coroutine_n; ++i) {
    ch >> signal;
  }

  auto ch_after = clk::now();
  auto ch_duration = ch_after - ch_before;
  auto ch_us = std::chrono::duration_cast<us>(ch_duration).count();

  fmt::print("receive {} signals from channel, cost {} us\n", coroutine_n, ch_us);
}

static void libgo_loop_test_1(int coroutine_n) {
  // For each element in data vector
  // create a thread to do multiply
  std::vector<int> datas(coroutine_n, 10);
  std::vector<int> results(coroutine_n);
  std::transform(datas.begin(), datas.end(), results.begin(), Utils::op_mul_1<int>);

  co_chan<int> ch;
  for (int i = 0; i < coroutine_n; ++i) {
    go[=, &datas]() {
      Utils::f_mul_1(&datas[i]);
      ch << 1;
    };
  };

  std::thread([]() { co_sched.Start(/*default = (1, 0)*/); }).detach();
  auto run_before = clk::now();

  int join;
  for (int i = 0; i < coroutine_n; i++) {
    ch >> join;
  }

  auto run_after = clk::now();
  auto run_duration = run_after - run_before;
  auto run_us = std::chrono::duration_cast<us>(run_duration).count();

  fmt::print(
      "launch {} threads to multiply a vector to a scalar, end-to-end cost {} us\n",
      coroutine_n, run_us);

  assert(datas == results);
}

static void libgo_loop_test_2(int coroutine_n) { // For each element in data vector
  // create a thread to do multiply
  std::vector<int> datas(coroutine_n, 10);
  std::vector<int> results(coroutine_n);
  std::transform(datas.begin(), datas.end(), results.begin(),
                 Utils::op_mul_1000000<int>);

  co_chan<int> ch;
  for (int i = 0; i < coroutine_n; ++i) {
    go[=, &datas]() {
      Utils::f_mul_1M(&datas[i]);
      ch << 1;
    };
  };

  std::thread([]() { co_sched.Start(std::thread::hardware_concurrency()); }).detach();
  auto run_before = clk::now();

  int join;
  for (int i = 0; i < coroutine_n; ++i) {
    ch >> join;
  }

  auto run_after = clk::now();
  auto run_duration = run_after - run_before;
  auto run_us = std::chrono::duration_cast<us>(run_duration).count();

  fmt::print("launch {} coroutine on {} threads to multiply a vector to a scalar, "
             "end-to-end cost {} us\n",
             coroutine_n, std::thread::hardware_concurrency(), run_us);

  assert(datas == results);
}

static void libgo_ctx_switch_test_1(uint64_t switch_n) {
  auto switch_before = new time_point_t{};
  auto switch_after = new time_point_t{};

  co_chan<int> ch;
  go[=]() {
    auto switch_left = switch_n;

    *switch_before = clk::now();
    while (switch_left--) {
      co_yield;
    }
    *switch_after = clk::now();

    ch << 1; // join
  };

  std::thread([]() { co_sched.Start(1); }).detach();

  int join;
  ch >> join;

  auto switch_duration = *switch_after - *switch_before;
  auto switch_us = std::chrono::duration_cast<us>(switch_duration).count();

  fmt::print("launch 1 coroutines, switch in-and-out {} times, cost {} us.\n",
             (uint64_t)switch_n, switch_us);

  delete switch_before;
  delete switch_after;
}

static void libgo_ctx_switch_test_2(int coroutine_n, uint64_t switch_n) {
  using time_point_t = decltype(clk::now());
  auto switch_befores = new time_point_t[coroutine_n];
  auto switch_afters = new time_point_t[coroutine_n];

  co_chan<int> ch;
  for (int i = 0; i < coroutine_n; ++i) {
    go[=]() {
      auto switch_left = switch_n;

      switch_befores[i] = clk::now();
      while (switch_left--) {
        co_yield;
      }
      switch_afters[i] = clk::now();

      ch << 1; // join
    };
  }

  std::thread([]() { co_sched.Start(std::thread::hardware_concurrency()); }).detach();

  int join;
  for (int i = 0; i < coroutine_n; ++i) {
    ch >> join;
  }

  auto switch_before = *std::min_element(switch_befores, switch_befores + coroutine_n);
  auto switch_after = *std::max_element(switch_afters, switch_afters + coroutine_n);
  auto switch_duration = switch_after - switch_before;
  auto switch_us = std::chrono::duration_cast<us>(switch_duration).count();

  fmt::print("launch {} coroutines, switch in-and-out {} times, cost {} us.\n",
             coroutine_n, (uint64_t)switch_n, switch_us);

  delete[] switch_befores;
  delete[] switch_afters;
}

static void libgo_start_urgent_test(int coroutine_n) {
  // This test is to emulate bthread_start_urgent. The worker routine starts a new
  // urgent routine an let it run on current thread. Then the worker are appending
  // to current thread's processing queue. If the urgent routine blocks or spending
  // long time. Then the work steal scheduler will steal worker to another thread
  // !! This won't actually have the same behaviour with bthread.
  //    bthread_start_urgent() swap the urgent bthread in and add worker bthread to
  //    processing queue. But go-and-co_yield just add the urgent routine and worker
  //    routine to processing queue. They behave the same only when the processing
  //    queue is empty.

  auto urgent_before = new time_point_t{};
  auto urgent_start = new time_point_t{};
  auto urgent_after = new time_point_t{};

  co_chan<int> ch;
  go[=]() {
    // fmt::print("worker tid: {}\n", pthread_self());
    *urgent_before = clk::now();

    go[=]() {
      *urgent_start = clk::now();
      // fmt::print("urgent tid: {}\n", pthread_self());
      // 1. Block the thread. Enable this only when libco compiled as no-hook
      // sleep(2);
      // 2. Use CPU for long calculation
      int i = 10;
      Utils::f_mul_1M(&i);
      fmt::print("{}\n", i);

      ch << 1;
    };
    co_yield;

    *urgent_after = clk::now();
    // fmt::print("worker tid: {}\n", pthread_self());

    ch << 1;
  };

  std::thread([]() { co_sched.Start(2); }).detach();

  int join;
  ch >> join, ch >> join;

  auto urgent_duration = *urgent_start - *urgent_before;
  auto worker_duration = *urgent_after - *urgent_before;
  auto urgent_us = std::chrono::duration_cast<us>(urgent_duration).count();
  auto worker_us = std::chrono::duration_cast<us>(worker_duration).count();
  fmt::print("cost {} us to schedule a new libgo::routine in current pthread.\n", urgent_us);
  fmt::print("cost {} us to schedule current libgo::routine to another pthread.\n", worker_us);

  delete urgent_before;
  delete urgent_start;
  delete urgent_after;
}

Benchmark benchmark{
    libgo_create_join_test,  libgo_loop_test_1,       libgo_loop_test_2,
    libgo_ctx_switch_test_1, libgo_ctx_switch_test_2, libgo_start_urgent_test,
};