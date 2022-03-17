#include "benchmark.h"
#include <algorithm>
#include <assert.h>
#include <chrono>
#include <fmt/core.h>
#include <iostream>
#include <pthread.h>
#include <vector>

// pthread start
static void pthread_create_join_test(int thread_n) {
  // Create thread_n threads
  std::vector<pthread_t> threads(thread_n);
  auto create_before = clk::now();
  for (auto &tid : threads) {
    pthread_create(&tid, nullptr, Utils::f_null, nullptr);
  }
  auto create_after = clk::now();
  auto create_duration = create_after - create_before;
  auto create_us = std::chrono::duration_cast<us>(create_duration).count();

  fmt::print("create {} threads, cost {} us\n", thread_n, create_us);

  // Join thread_n threads
  auto join_before = clk::now();
  for (auto tid : threads) {
    pthread_join(tid, nullptr);
  }
  auto join_after = clk::now();
  auto join_duration = join_after - join_before;
  auto join_us = std::chrono::duration_cast<us>(join_duration).count();
  fmt::print("join {} threads, cost {} us\n", thread_n, join_us);
}

static void pthread_loop_test_1(int thread_n) {
  std::vector<int> datas(thread_n, 10);
  std::vector<int> results(thread_n);
  std::transform(datas.begin(), datas.end(), results.begin(), Utils::op_mul_1<int>);

  std::vector<pthread_t> threads(thread_n);

  auto run_before = clk::now();
  for (int i = 0; i < thread_n; ++i) {
    pthread_create(&threads[i], nullptr, Utils::f_mul_1, &datas[i]);
  }
  for (auto tid : threads) {
    pthread_join(tid, nullptr);
  }
  auto run_after = clk::now();
  auto run_duration = run_after - run_before;
  auto run_us = std::chrono::duration_cast<us>(run_duration).count();

  assert(datas == results);

  fmt::print(
      "launch {} threads to multiply a vector to a scalar, end-to-end cost {} us\n",
      thread_n, run_us);
}

static void pthread_loop_test_2(int thread_n) {
  std::vector<int> datas(thread_n, 10);
  // std::vector<int> results(thread_n);
  // std::transform(datas.begin(), datas.end(), results.begin(),
  //                Utils::op_mul_1000000<int>);

  std::vector<pthread_t> threads(thread_n);

  auto run_before = clk::now();
  for (int i = 0; i < thread_n; ++i) {
    pthread_create(&threads[i], nullptr, Utils::f_mul_1000000, &datas[i]);
  }
  for (auto tid : threads) {
    pthread_join(tid, nullptr);
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
    pthread_yield();
  }
  switch_afters[thread_i] = clk::now();

  return nullptr;
}

// not accurate !!!
static void pthread_ctx_switch_test_1(uint64_t switch_n) {
  // timers.
  auto switch_before = new time_point_t{};
  auto switch_after = new time_point_t{};

  // args
  auto arg = new args_ctx_switch_t{0, switch_n, switch_before, switch_after};

  // threads
  auto tid = pthread_t{};
  pthread_create(&tid, nullptr, f_ctx_switch, &arg);
  pthread_join(tid, nullptr);

  auto switch_duration = *switch_after - *switch_before;
  auto switch_us = std::chrono::duration_cast<us>(switch_duration).count();

  fmt::print("launch 1 threads, switch in-and-out {} times, cost {} us.\n",
             (uint64_t)switch_n, switch_us);

  delete switch_before;
  delete switch_after;
  delete arg;
}

static void pthread_ctx_switch_test_2(int thread_n, uint64_t switch_n) {
  // timers.
  auto switch_befores = new time_point_t[thread_n];
  auto switch_afters = new time_point_t[thread_n];

  // args
  auto args = new args_ctx_switch_t[thread_n];
  for (int i = 0; i < thread_n; ++i) {
    args[i] = {i, switch_n, switch_befores, switch_afters};
  }

  // threads
  auto threads = std::vector<pthread_t>(thread_n);
  for (int i = 0; i < thread_n; ++i) {
    pthread_create(&threads[i], nullptr, f_ctx_switch, &args[i]);
  }
  for (auto tid : threads) {
    pthread_join(tid, nullptr);
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

static void pthread_long_callback_test(int thread_n) {
  // TODO
}
// pthread end
Benchmark benchmark{
    pthread_create_join_test,  pthread_loop_test_1,       pthread_loop_test_2,
    pthread_ctx_switch_test_1, pthread_ctx_switch_test_2, pthread_long_callback_test,
};