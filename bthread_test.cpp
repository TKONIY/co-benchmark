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
static void bthread_loop_test(int thread_n) {
  // For each element in data vector
  // create a thread to do multiply
  std::vector<int> datas(thread_n, 10);
  std::vector<int> results(thread_n);
  std::transform(datas.begin(), datas.end(), results.begin(), Utils::op_mul<int>);

  std::vector<bthread_t> threads(thread_n);

  auto run_before = clk::now();
  for (int i = 0; i < thread_n; ++i) {
    bthread_start_background(&threads[i], nullptr, Utils::f_mul, &datas[i]);
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
static void bthread_ctx_switch_test(int thread_n, uint64_t) {
  // TODO 参考Imbench
}
static void bthread_long_callback_test(int thread_n) {
  // TODO 测试长尾
}

Benchmark benchmark{
    bthread_create_join_test,
    bthread_loop_test,
    bthread_ctx_switch_test,
    bthread_long_callback_test,
};