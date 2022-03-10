#include "benchmark.h"
#include <assert.h>
#include <chrono>
#include <fmt/core.h>
#include <iostream>
#include <libgo/coroutine.h>

static void libgo_create_join_test(int thread_n) {
  go[] {
    while (true) {
      fmt::print("coroutine before sleep\n");
      co_sleep(1000);
      fmt::print("coroutine after sleep\n");
    }
  };

  co_sched.Start();
  // co_sleep(100);
  // while (true) {
  //   fmt::print("caller before sleep\n");
  //   (1000);
  //   fmt::print("caller after sleep\n");
  // }
}
static void libgo_loop_test(int thread_n) {}
static void libgo_ctx_switch_test(int thread_n, uint64_t) {
  // TODO 参考Imbench
}
static void libgo_long_callback_test(int thread_n) {
  // TODO 测试长尾
}

Benchmark benchmark{
    libgo_create_join_test,
    libgo_loop_test,
    libgo_ctx_switch_test,
    libgo_long_callback_test,
};