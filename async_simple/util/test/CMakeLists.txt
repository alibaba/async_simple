file(GLOB util_test_src "*.cpp")
add_executable(async_simple_util_test ${util_test_src} ${PROJECT_SOURCE_DIR}/async_simple/test/dotest.cpp)

target_link_libraries(async_simple_util_test async_simple ${deplibs} ${testdeplibs})

add_test(NAME run_async_simple_util_test COMMAND async_simple_util_test)
