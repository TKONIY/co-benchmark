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

static void bthread_ctx_switch_test(int thread_n, uint64_t switch_n) {
  // TODO 参考Imbench
  // timers.
  auto switch_befores = std::vector<time_point_t>(thread_n);
  auto switch_afters = std::vector<time_point_t>(thread_n);

  // args
  auto args = std::vector<args_ctx_switch_t>(thread_n);
  for (int i = 0; i < thread_n; ++i) {
    args[i] = {i, switch_n, switch_befores.data(), switch_afters.data()};
  }

  // threads
  auto threads = std::vector<bthread_t>(thread_n);
  for (int i = 0; i < thread_n; ++i) {
    bthread_start_background(&threads[i], nullptr, f_ctx_switch, &args[i]);
  }
  for (auto tid : threads) {
    bthread_join(tid, nullptr);
  }

  auto switch_before = *std::min_element(switch_befores.begin(), switch_befores.end());
  auto switch_after = *std::max_element(switch_afters.begin(), switch_afters.end());
  auto switch_duration = switch_after - switch_before;
  auto switch_us = std::chrono::duration_cast<us>(switch_duration).count();

  fmt::print("launch {} threads, switch in-and-out {} times, cost {} us.\n", thread_n,
             (uint64_t)switch_n, switch_us);
}
static void bthread_long_callback_test(int thread_n) {
  // TODO 测试长尾
}

Benchmark benchmark{
    bthread_create_join_test, bthread_loop_test_1,        bthread_loop_test_2,
    bthread_ctx_switch_test,  bthread_long_callback_test,
};