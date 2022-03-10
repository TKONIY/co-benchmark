#include "benchmark.h"
#include <cassert>
#include <coroutine>
#include <fmt/core.h>
#include <memory>
#include <vector>

// the same coroutine semantic as libco
// initial_suspend -> std::suspend_always:
//     libco needs resume() after create;
// final_suspend   -> std::suspend_always:
//     libco needs release() after finish;
struct promise;
struct coroutine : std::coroutine_handle<promise> {
  using promise_type = struct promise;
};

struct promise {
  coroutine get_return_object() { return {coroutine::from_promise(*this)}; }
  std::suspend_always initial_suspend() noexcept { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  void return_void() {}
  void unhandled_exception() {}
};
// coroutine semantic

void cpp20co_create_join_test(int coroutine_n) {
  std::vector<coroutine> coroutines(coroutine_n);
  auto create_before = clk::now();
  for (auto &tid : coroutines) {
    tid = []() -> coroutine {
      Utils::f_null(nullptr);
      co_return;
    }();
  }
  auto create_after = clk::now();
  auto create_duration = create_after - create_before;
  auto create_us = std::chrono::duration_cast<us>(create_duration).count();
  auto create_ns = std::chrono::duration_cast<ns>(create_duration).count();
  fmt::print("create {} coroutines, cost {} us, {} ns\n", coroutine_n, create_us,
             create_ns);

  auto resume_before = clk::now();
  for (auto &tid : coroutines) {
    tid.resume();
  }
  auto resume_after = clk::now();
  auto resume_duration = resume_after - resume_before;
  auto resume_us = std::chrono::duration_cast<us>(resume_duration).count();
  auto resume_ns = std::chrono::duration_cast<ns>(resume_duration).count();
  fmt::print("resume {} coroutines, cost {} us, {} ns\n", coroutine_n, resume_us,
             resume_ns);

  // destroy by RAII
}

void cpp20co_loop_test(int coroutine_n) {
  std::vector<int> datas(coroutine_n, 10);
  std::vector<int> results(coroutine_n);
  std::transform(datas.begin(), datas.end(), results.begin(), Utils::op_mul<int>);

  std::vector<coroutine> coroutines(coroutine_n);

  auto run_before = clk::now();
  for (int i = 0; i < coroutine_n; ++i) {
    coroutines[i] = [&]() -> coroutine {
      Utils::f_mul(&datas[i]);
      co_return;
    }();
    // co_resume(coroutines[i]);

    // @notes:
    // co_resume() can be put under or here, it's almost the same,
    // since libco only runs in one CPU core.
  }
  for (auto &tid : coroutines) {
    tid.resume();
  }
  auto run_after = clk::now();
  auto run_duration = run_after - run_before;
  auto run_us = std::chrono::duration_cast<us>(run_duration).count();

  assert(datas == results);

  fmt::print(
      "launch {} coroutines to multiply a vector to a scalar, end-to-end cost {} us\n",
      coroutine_n, run_us);
}

void cpp20co_ctx_switch_test(int coroutine_n, uint64_t switch_n) {
  auto co = [](uint64_t switch_n) -> coroutine {
    while (switch_n--) {
      co_await std::suspend_always{};
    }
    co_return;
  }(switch_n);

  auto switch_before = clk::now();

  auto switch_left = switch_n;
  while (switch_left--) {
    co.resume();
  }

  auto switch_after = clk::now();
  auto switch_duration = switch_after - switch_before;
  auto switch_us = std::chrono::duration_cast<us>(switch_duration).count();

  fmt::print("launch 1 coroutines, switch in-and-out {} times, cost {} us.\n",
             (uint64_t)switch_n, switch_us);

  // co destroyed by RAII
}

void cpp20co_long_callback_test(int coroutine_n) {}
// cpp20co end
Benchmark benchmark{
    cpp20co_create_join_test,
    cpp20co_loop_test,
    cpp20co_ctx_switch_test,
    cpp20co_long_callback_test,
};