#include "benchmark.h"
#include <assert.h>
#include <chrono>
#include <fmt/core.h>
#include <iostream>
#include <libgo/coroutine.h>

static void libgo_create_join_test(int thread_n) {
  auto create_before = clk::now();

  co_chan<int> ch;
  for (int i = 0; i < thread_n; ++i) {
    go[ch] {
      Utils::f_null(nullptr);
      ch << 1;
    };
  }

  auto create_after = clk::now();
  auto create_duration = create_after - create_before;
  auto create_us = std::chrono::duration_cast<us>(create_duration).count();

  fmt::print("create {} threads, cost {} us\n", thread_n, create_us);

  // ignore new schedule thread's overhead
  std::thread([]() { co_sched.Start(); }).detach();

  auto ch_before = clk::now();

  int signal;
  for (int i = 0; i < thread_n; ++i) {
    ch >> signal;
  }

  auto ch_after = clk::now();
  auto ch_duration = ch_after - ch_before;
  auto ch_us = std::chrono::duration_cast<us>(ch_duration).count();

  fmt::print("receive {} signals from channel, cost {} us\n", thread_n, ch_us);
}
static void libgo_loop_test_1(int thread_n) { fmt::print("Not implemented.\n"); }
static void libgo_loop_test_2(int thread_n) { fmt::print("Not implemented.\n"); }
static void libgo_ctx_switch_test(int thread_n, uint64_t switch_n) {
  using time_point_t = decltype(clk::now());
  auto switch_befores = std::vector<time_point_t>(thread_n);
  auto switch_afters = std::vector<time_point_t>(thread_n);

  co_chan<int> ch;
  for (int i = 0; i < thread_n; ++i) {
    go[=, &switch_befores, &switch_afters]() {
      auto switch_left = switch_n;

      switch_befores[i] = clk::now();
      while (switch_left--) {
        co_yield;
      }
      switch_afters[i] = clk::now();

      ch << 1; // join
    };
  }

  std::thread([]() { co_sched.Start(); }).detach();

  int join;
  for (int i = 0; i < thread_n; ++i) {
    ch >> join;
  }

  auto switch_before = *std::min_element(switch_befores.begin(), switch_befores.end());
  auto switch_after = *std::max_element(switch_afters.begin(), switch_afters.end());
  auto switch_duration = switch_after - switch_before;
  auto switch_us = std::chrono::duration_cast<us>(switch_duration).count();

  fmt::print("launch {} coroutines, switch in-and-out {} times, cost {} us.\n",
             thread_n, (uint64_t)switch_n, switch_us);
}
static void libgo_long_callback_test(int thread_n) {
  // TODO 测试长尾
}

Benchmark benchmark{
    libgo_create_join_test, libgo_loop_test_1,        libgo_loop_test_2,
    libgo_ctx_switch_test,  libgo_long_callback_test,
};