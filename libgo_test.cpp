#include "benchmark.h"
#include <assert.h>
#include <chrono>
#include <fmt/core.h>
#include <iostream>
#include <libgo/coroutine.h>

static void libgo_create_join_test(int thread_n) {
  auto create_before = clk::now();

  for (int i = 0; i < thread_n; ++i) {
    go[] {
      Utils::f_null(nullptr);
      // while (true) {
      //   fmt::print("coroutine before sleep\n");
      //   co_sleep(1000);
      //   fmt::print("coroutine after sleep\n");
      // }
    };
  }

  auto create_after = clk::now();
  auto create_duration = create_after - create_before;
  auto create_us = std::chrono::duration_cast<us>(create_duration).count();

  fmt::print("create {} threads, cost {} us\n", thread_n, create_us);

  co_sched.Start();
  // co_sleep(100);
  // while (true) {
  //   fmt::print("caller before sleep\n");
  //   (1000);
  //   fmt::print("caller after sleep\n");
  // }
}
static void libgo_loop_test_1(int thread_n) { fmt::print("Not implemented.\n"); }
static void libgo_loop_test_2(int thread_n) { fmt::print("Not implemented.\n"); }
static void libgo_ctx_switch_test(int thread_n, uint64_t) {
  // TODO 参考Imbench
}
static void libgo_long_callback_test(int thread_n) {
  // TODO 测试长尾
}

Benchmark benchmark{
    libgo_create_join_test, libgo_loop_test_1,        libgo_loop_test_2,
    libgo_ctx_switch_test,  libgo_long_callback_test,
};