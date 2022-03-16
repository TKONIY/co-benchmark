#include <chrono>
#include <sys/sysinfo.h>
// global definitions start
using clk = std::chrono::system_clock;
using ms = std::chrono::milliseconds;
using us = std::chrono::microseconds;
using ns = std::chrono::nanoseconds;
using s = std::chrono::seconds;

struct Utils {
  static void *f_null(void *) { return nullptr; }
  static void *f_mul_1(void *value) {
    // multiply
    *static_cast<int *>(value) *= 10;
    return nullptr;
  }
  static void *f_mul_1000000(void *value) {
    auto value_int = *static_cast<int *>(value);
    for (int i = 0; i < 1000000; ++i) {
      value_int = value_int * 10;
    }
    *static_cast<int *>(value) = value_int;
    return nullptr;
  }
  
  template <typename T> static T op_mul_1(T value) { return value * 10; }
  template <typename T> static T op_mul_1000000(T value) {
    for (int i = 0; i < 1000000; ++i) {
      value = value * 10;
    }
    return value;
  }

  static int n_cpu() {
    return get_nprocs();
  }
};

struct Benchmark {
  void (*create_join_test)(int thread_n);
  void (*loop_test_1)(int thread_n);
  void (*loop_test_2)(int thread_n);
  void (*ctx_switch_test)(int thread_n, uint64_t switch_n);
  void (*long_callback_test)(int thread_n);
};
// global definitions end
