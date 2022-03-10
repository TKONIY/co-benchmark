#include <chrono>
// global definitions start
using clk = std::chrono::system_clock;
using ms = std::chrono::milliseconds;
using us = std::chrono::microseconds;
using ns = std::chrono::nanoseconds;
using s = std::chrono::seconds;

struct Utils {
  static void *f_null(void *) { return nullptr; }
  static void *f_mul(void *value) {
    // multiply
    *static_cast<int *>(value) *= 10;
    return nullptr;
  }
  template <typename T> static T op_mul(T value) { return value * 10; }
};

struct Benchmark {
  void (*create_join_test)(int thread_n);
  void (*loop_test)(int thread_n);
  void (*ctx_switch_test)(int thread_n, uint64_t switch_n);
  void (*long_callback_test)(int thread_n);
};
// global definitions end
