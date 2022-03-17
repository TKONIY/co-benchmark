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
  std::thread([]() { co_sched.Start(Utils::n_cpu()); }).detach();

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

  std::thread([]() { co_sched.Start(/*default*/); }).detach();
  auto run_before = clk::now();

  int join;
  for (int i = 0; i < coroutine_n; i++) {
    ch >> join;
  }

  auto run_after = clk::now();
  auto run_duration = run_after - run_before;
  auto run_us = std::chrono::duration_cast<us>(run_duration).count();

  assert(datas == results);

  fmt::print(
      "launch {} threads to multiply a vector to a scalar, end-to-end cost {} us\n",
      coroutine_n, run_us);
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
      Utils::f_mul_1000000(&datas[i]);
      ch << 1;
    };
  };

  std::thread([]() { co_sched.Start(Utils::n_cpu()); }).detach();
  auto run_before = clk::now();

  int join;
  for (int i = 0; i < coroutine_n; ++i) {
    ch >> join;
  }

  auto run_after = clk::now();
  auto run_duration = run_after - run_before;
  auto run_us = std::chrono::duration_cast<us>(run_duration).count();

  assert(datas == results);

  fmt::print("launch {} coroutine on {} threads to multiply a vector to a scalar, "
             "end-to-end cost {} us\n",
             coroutine_n, Utils::n_cpu(), run_us);
}

static void libgo_ctx_switch_test_1(uint64_t switch_n) {
  using time_point_t = decltype(clk::now());
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

  std::thread([]() { co_sched.Start(Utils::n_cpu()); }).detach();

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

static void libgo_long_callback_test(int coroutine_n) {
  // TODO 测试长尾
}

Benchmark benchmark{
    libgo_create_join_test,  libgo_loop_test_1,       libgo_loop_test_2,
    libgo_ctx_switch_test_1, libgo_ctx_switch_test_2, libgo_long_callback_test,
};