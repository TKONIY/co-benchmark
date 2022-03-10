# Thread and Coroutines Benchmark
I'm not going to argue with anyone about the fucking confusing concepts of thread, coroutine and fiber. The following articles are devided by stackful and stackless. Futher symmetric and asymmetric will also be distinguish.
## TODO
- [ ] Rewrite CMakeLists.txt to find brpc in system library.
- [ ] Design benchmark for stackful and stackless seperately.
## Benchmarks

|              | bthread | pthread | libco | cpp20co |
| ------------ | ------- | ------- | ----- | ------- |
| create       | done    | done    | done  | done    |
| join/resume  | done    | done    | done  | done    |
| loop         | done    | done    | done  | done    |
| ctx_switch   |         |         |       |         |
| long_callback |         |         |       |         |