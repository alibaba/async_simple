add_executable(Lazy.bench Lazy.bench.cpp)
target_link_libraries(Lazy.bench ${BENCHMARK_LIBRARIES} async_simple)
