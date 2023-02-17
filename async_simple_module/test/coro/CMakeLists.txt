file(GLOB test_src "*.cpp")
add_executable(async_simple_coro_test ../dotest.cpp ${test_src})

target_link_libraries(async_simple_coro_test async_simple ${deplibs} ${testdeplibs})

add_test(NAME run_async_simple_test COMMAND async_simple_coro_test)
