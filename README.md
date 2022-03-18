# Thread and Coroutines Benchmark
I'm not going to argue with anyone about the fucking confusing concepts of thread, coroutine and fiber. The following articles will be devided by stackful and stackless. Futher symmetric and asymmetric routines will also be distinguished.
## TODO
- [ ] Rewrite CMakeLists.txt to find brpc in system library.
- [ ] Design benchmark for stackful and stackless seperately.
## Benchmarks
### Common Benchmarks
|                          | bthread | pthread | libco | cpp20co | libgo |
| ------------------------ | ------- | ------- | ----- | ------- | ----- |
| create                   | ✅       | ✅       | ✅     | ✅       | ✅     |
| join                     | ✅       | ✅       | 🈚️     | 🈚️       | ✅     |
| resume                   | 🈚️       | 🈚️       | ✅     | ✅       | 🈚️     |
| multiply 1               | ✅       | ✅       | ✅     | ✅       | ✅     |
| multiply 1M              | ✅       | ✅       | ✅     | ✅       | ✅     |
| ctx switch single-thread | ✅       | ✅       | ✅     | ✅       | ✅     |
| ctx switch multi-thread  | ✅       | ✅       | 🈚️     | 🈚️       | ✅     |
### Library Specific Benchmarks
#### Start Urgent Test
* bthread: `bthread_start_urgent`
* libgo: `go` and `yield`
## Getting Start
> ⚠️ Since I haven't figured out how to write the CMakeLists.txt for projects with brpc, current build configurations of this project are taken from `incubator-brpc/example/echo_c++`. Thus this project can only be place on the same dir. This will be fix as soon as possible.
> 
> ⚠️ Different benchmarks can be switched by modifying `main()` in `benchmark.cpp`. Command line support will be added soon.
1. Install all libraries into default system path except **brpc**. CMake looks up the libraries in default path.
2. Clone and compile brpc according to official [tutorial](https://github.com/apache/incubator-brpc/blob/master/docs/cn/getting_started.md).
3. Clone this project into brpc's example dir.
   ```shell
   cd incubator-brpc/example
   git clone git@github.com:TKONIY/ThreadBenchmark.git
   ```
4. Modify `main()` in `benchmark.cpp`. One benchmark of all libraries will be provided in one build. 
5. Build the project.
  ```shell
  cd ThreadBenchmark
  mkdir build
  cd build
  cmake ..
  cmake --build .
  ```
6. All executable will be provided in `build/` like this:
  ```txt
  build
    ├── benchmark_bthread
    ├── benchmark_cpp20co
    ├── benchmark_libco
    ├── benchmark_libgo
    └── benchmark_pthread
  ```
7. Just simply execute the binary files.
  ```shell
  ./benchmark_cpp20co
  ```
  