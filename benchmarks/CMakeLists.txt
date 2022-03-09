add_executable(benchmarking
    Lazy.bench.cpp
    Future.bench.cpp
    Uthread.bench.cpp
    benchmark_main.cpp)
target_link_libraries(benchmarking ${BENCHMARK_LIBRARIES} async_simple)
