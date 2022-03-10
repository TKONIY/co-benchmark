#include "benchmark.h"
#include <algorithm>
#include <cassert>
#include <fmt/core.h>
#include <libco/co_routine.h>
#include <vector>

static void libco_create_join_test(int coroutine_n) {
  // create coroutine_n coroutines
  std::vector<stCoRoutine_t *> coroutines(coroutine_n);
  auto create_before = clk::now();
  for (auto &tid : coroutines) {
    co_create(&tid, nullptr, Utils::f_null, nullptr);
  }
  auto create_after = clk::now();
  auto create_duration = create_after - create_before;
  auto create_us = std::chrono::duration_cast<us>(create_duration).count();
  auto create_ns = std::chrono::duration_cast<ns>(create_duration).count();
  fmt::print("create {} coroutines, cost {} us, {} ns\n", coroutine_n, create_us,
             create_ns);

  auto resume_before = clk::now();
  for (auto &tid : coroutines) {
    co_resume(tid);
  }
  auto resume_after = clk::now();
  auto resume_duration = resume_after - resume_before;
  auto resume_us = std::chrono::duration_cast<us>(resume_duration).count();
  auto resume_ns = std::chrono::duration_cast<ns>(resume_duration).count();
  fmt::print("resume {} coroutines, cost {} us, {} ns\n", coroutine_n, resume_us,
             resume_ns);
}

static void libco_loop_test(int coroutine_n) {
  std::vector<int> datas(coroutine_n, 10);
  std::vector<int> results(coroutine_n);
  std::transform(datas.begin(), datas.end(), results.begin(), Utils::op_mul<int>);

  std::vector<stCoRoutine_t *> coroutines(coroutine_n);

  auto run_before = clk::now();
  for (int i = 0; i < coroutine_n; ++i) {
    co_create(&coroutines[i], nullptr, Utils::f_mul, &datas[i]);
    // co_resume(coroutines[i]);

    // @notes:
    // co_resume() can be put under or here, it's almost the same,
    // since libco only runs in one CPU core.
  }
  for (auto &tid : coroutines) {
    co_resume(tid);
  }
  auto run_after = clk::now();
  auto run_duration = run_after - run_before;
  auto run_us = std::chrono::duration_cast<us>(run_duration).count();

  assert(datas == results);

  fmt::print(
      "launch {} coroutines to multiply a vector to a scalar, end-to-end cost {} us\n",
      coroutine_n, run_us);
}

static void libco_ctx_switch_test(int coroutine_n) {
  // TODO
}

static void libco_long_callback_test(int coroutine_n) {
  // TODO
}

Benchmark benchmark{
    libco_create_join_test,
    libco_loop_test,
    libco_ctx_switch_test,
    libco_long_callback_test,
};