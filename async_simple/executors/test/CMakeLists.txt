file(GLOB executor_test_src "*.cpp")
add_executable(async_simple_executor_test ${executor_test_src} ${PROJECT_SOURCE_DIR}/async_simple/test/dotest.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_link_libraries(async_simple_executor_test async_simple ${deplibs} ${testdeplibs})
else()
    target_link_libraries(async_simple_executor_test async_simple -pthread ${deplibs} ${testdeplibs})
endif()

add_test(NAME run_async_simple_executor_test COMMAND async_simple_executor_test)

