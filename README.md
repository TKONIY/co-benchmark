# Thread and Coroutines Benchmark
## TODO
- [ ] Rewrite CMakeLists.txt to find brpc in system library.
- [ ] Design benchmark for coroutine and thread seperately.
## Progress
|              | bthread | pthread | libco |
| ------------ | ------- | ------- | ----- |
| create       | done    | done    | done  |
| join/resume  | done    | done    | done  |
| loop         | done    | done    | done  |
| ctx_switch   |         |         |       |
| long_calback |         |         |       |