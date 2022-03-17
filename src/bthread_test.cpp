#include "benchmark.h"
#include <assert.h>
#include <bthread/bthread.h>
#include <chrono>
#include <fmt/core.h>
#include <iostream>

static void bthread_create_join_test(int thread_n) {
  // Create thread_n threads
  std::vector<bthread_t> threads(thread_n);
  auto create_before = clk::now();
  for (auto &tid : threads) {
    bthread_start_background(&tid, nullptr, Utils::f_null, nullptr);
  }
  auto create_after = clk::now();
  auto create_duration = create_after - create_before;
  auto create_us = std::chrono::duration_cast<us>(create_duration).count();

  fmt::print("create {} threads, cost {} us\n", thread_n, create_us);

  // Join thread_n threads
  auto join_before = clk::now();
  for (auto tid : threads) {
    bthread_join(tid, NULL);
  }
  auto join_after = clk::now();
  auto join_duration = join_after - join_before;
  auto join_us = std::chrono::duration_cast<us>(join_duration).count();
  fmt::print("join {} threads, cost {} us\n", thread_n, join_us);
}
static void bthread_loop_test_1(int thread_n) {
  // For each element in data vector
  // create a thread to do multiply
  std::vector<int> datas(thread_n, 10);
  std::vector<int> results(thread_n);
  std::transform(datas.begin(), datas.end(), results.begin(), Utils::op_mul_1<int>);

  std::vector<bthread_t> threads(thread_n);

  auto run_before = clk::now();
  for (int i = 0; i < thread_n; ++i) {
    bthread_start_background(&threads[i], nullptr, Utils::f_mul_1, &datas[i]);
  }
  for (auto tid : threads) {
    bthread_join(tid, NULL);
  }
  auto run_after = clk::now();
  auto run_duration = run_after - run_before;
  auto run_us = std::chrono::duration_cast<us>(run_duration).count();

  assert(datas == results);

  fmt::print(
      "launch {} threads to multiply a vector to a scalar, end-to-end cost {} us\n",
      thread_n, run_us);
}
static void bthread_loop_test_2(int thread_n) {
  // For each element in data vector
  // create a thread to do multiply
  std::vector<int> datas(thread_n, 10);
  // std::vector<int> results(thread_n);
  // std::transform(datas.begin(), datas.end(), results.begin(),
  //                Utils::op_mul_1000000<int>);

  std::vector<bthread_t> threads(thread_n);

  auto run_before = clk::now();
  for (int i = 0; i < thread_n; ++i) {
    bthread_start_background(&threads[i], nullptr, Utils::f_mul_1000000, &datas[i]);
  }
  for (auto tid : threads) {
    bthread_join(tid, NULL);
  }
  auto run_after = clk::now();
  auto run_duration = run_after - run_before;
  auto run_us = std::chrono::duration_cast<us>(run_duration).count();

  // assert(datas == results);

  fmt::print(
      "launch {} threads to multiply a vector to a scalar, end-to-end cost {} us\n",
      thread_n, run_us);
}

// ctx switch tests
using time_point_t = decltype(clk::now());

struct args_ctx_switch_t {
  int thread_i;
  uint64_t switch_n;
  time_point_t *switch_befores;
  time_point_t *switch_afters;
};

static void *f_ctx_switch(void *args) {
  auto args_ctx = static_cast<args_ctx_switch_t *>(args);
  auto thread_i = args_ctx->thread_i;
  auto switch_n = args_ctx->switch_n;
  auto switch_befores = args_ctx->switch_befores;
  auto switch_afters = args_ctx->switch_afters;

  switch_befores[thread_i] = clk::now();
  while (switch_n--) {
    bthread_yield();
  }
  switch_afters[thread_i] = clk::now();

  return nullptr;
}

static void bthread_ctx_switch_test_1(uint64_t switch_n) {
  // timers.
  auto switch_before = new time_point_t{};
  auto switch_after = new time_point_t{};

  // args
  auto arg = new args_ctx_switch_t{0, switch_n, switch_before, switch_after};

  // thread
  pthread_t tid{};
  bthread_start_background(&tid, nullptr, f_ctx_switch, arg);
  bthread_join(tid, nullptr);

  auto switch_duration = *switch_after - *switch_before;
  auto switch_us = std::chrono::duration_cast<us>(switch_duration).count();

  fmt::print("launch 1 threads, switch in-and-out {} times, cost {} us.\n",
             (uint64_t)switch_n, switch_us);
  
  delete switch_before;
  delete switch_after;
  delete arg;
}

static void bthread_ctx_test_2(int thread_n, uint64_t switch_n) {
  // timers.
  auto switch_befores = new time_point_t[thread_n];
  auto switch_afters = new time_point_t[thread_n];

  // args
  auto args = new args_ctx_switch_t[thread_n];
  for (int i = 0; i < thread_n; ++i) {
    args[i] = {i, switch_n, switch_befores, switch_afters};
  }

  // threads
  auto threads = std::vector<bthread_t>(thread_n);
  for (int i = 0; i < thread_n; ++i) {
    bthread_start_background(&threads[i], nullptr, f_ctx_switch, &args[i]);
  }
  for (auto tid : threads) {
    bthread_join(tid, nullptr);
  }

  auto switch_before = *std::min_element(switch_befores, switch_befores + thread_n);
  auto switch_after = *std::max_element(switch_afters, switch_afters + thread_n);
  auto switch_duration = switch_after - switch_before;
  auto switch_us = std::chrono::duration_cast<us>(switch_duration).count();

  fmt::print("launch {} threads, switch in-and-out {} times, cost {} us.\n", thread_n,
             (uint64_t)switch_n, switch_us);

  delete[] switch_befores;
  delete[] switch_afters;
  delete[] args;
}

// bthread start urgent test
struct arg_urgent_t {
  time_point_t urgent_start;
};

struct arg_worker_t {
  time_point_t urgent_before;
  time_point_t urgent_after;
};

static void *f_urgent(void *arg) {
  // fmt::print("urgent tid: {}\n", pthread_self());
  auto urgent_start = clk::now();
  // Block the pthread, then scheduler will steal bthread's on current TaskGroup to
  // other pthread. Stealing won't happened if f_urgent finish quickly.
  sleep(1);

  static_cast<arg_urgent_t *>(arg)->urgent_start = urgent_start;
  return nullptr;
}

static void *f_worker(void *arg) {
  bthread_t tid{};
  // fmt::print("worker tid before bthread_start_urgent(): {}\n", pthread_self());
  auto urgent_before = clk::now();
  bthread_start_urgent(&tid, nullptr, f_urgent, nullptr);
  auto urgent_after = clk::now();
  // fmt::print("worker tid after bthread_start_urgent(): {}\n", pthread_self());
  bthread_join(tid, nullptr);

  static_cast<arg_worker_t *>(arg)->urgent_before = urgent_before;
  static_cast<arg_worker_t *>(arg)->urgent_after = urgent_after;
  return nullptr;
}

static void bthread_start_urgent_test(int thread_n) {
  // Start a thread to run on current kernel.
  // The thread should be started in a worker thread, who have a thread-local,
  // TaskGroup by calling bthread_start_urgent(). Otherwise it will create a new pthread
  // to init TaskGroup, calling the same functixwon as bthread_start_background()
  // called.

  // arguments

  bthread_t tid{};
  // create a new TaskGroup to run f_worker
  bthread_start_background(&tid, nullptr, f_worker, nullptr);
  bthread_join(tid, nullptr);
}

Benchmark benchmark{
    bthread_create_join_test,  bthread_loop_test_1, bthread_loop_test_2,
    bthread_ctx_switch_test_1, bthread_ctx_test_2,  bthread_start_urgent_test,
};