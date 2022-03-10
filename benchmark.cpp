// bthread start
// bthread end
#include "benchmark.h"
#include <iostream>
extern Benchmark benchmark;
int main() {
  benchmark.create_join_test(1);
}