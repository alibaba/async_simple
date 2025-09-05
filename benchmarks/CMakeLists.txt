set(BENCHMARK_SRCS
        benchmark_main.cpp
        PureSwitch.bench.cpp
        ReadFile.bench.cpp
        CallDepth.bench.cpp
        Lazy.bench.cpp
        Future.bench.cpp
        ThreadPool.bench.cpp
        Mutex.bench.cpp
        SpinLock.bench.cpp
        SharedMutex.bench.cpp)

if (UTHREAD)
  list(APPEND BENCHMARK_SRCS Uthread.bench.cpp)
  add_compile_definitions(ASYNC_SIMPLE_BENCHMARK_UTHREAD)
endif()

add_executable(benchmarking ${BENCHMARK_SRCS})
target_link_libraries(benchmarking ${BENCHMARK_LIBRARIES} async_simple)

if(NOT ASYNC_SIMPLE_DISABLE_AIO AND LIBAIO_INCLUDE_DIR AND LIBAIO_LIBRARIES)
  target_link_libraries(benchmarking ${LIBAIO_LIBRARIES})
endif()
