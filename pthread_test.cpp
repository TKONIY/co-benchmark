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
    pthread_join(tid, NULL);
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
    pthread_join(tid, NULL);
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
    pthread_join(tid, NULL);
  }
  auto run_after = clk::now();
  auto run_duration = run_after - run_before;
  auto run_us = std::chrono::duration_cast<us>(run_duration).count();

  // assert(datas == results);

  fmt::print(
      "launch {} threads to multiply a vector to a scalar, end-to-end cost {} us\n",
      thread_n, run_us);
}

static void pthread_ctx_switch_test(int thread_n, uint64_t) {}

static void pthread_long_callback_test(int thread_n) {
  // TODO
}
// pthread end
Benchmark benchmark{
    pthread_create_join_test, pthread_loop_test_1,        pthread_loop_test_2,
    pthread_ctx_switch_test,  pthread_long_callback_test,
};