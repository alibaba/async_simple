file(GLOB coro_test_src "*.cpp")
add_executable(async_simple_coro_test ${coro_test_src} ${PROJECT_SOURCE_DIR}/async_simple/test/dotest.cpp)

target_link_libraries(async_simple_coro_test async_simple ${deplibs} ${testdeplibs})

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15.0" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0")
    # Clang gives incorrect warnings, See https://github.com/llvm/llvm-project/issues/56768
    message("Applying -Wno-unsequenced to async_simple_coro_test for Clang 15.x")
    target_compile_options(async_simple_coro_test PRIVATE -Wno-unsequenced)
endif()

add_test(NAME run_async_simple_coro_test COMMAND async_simple_coro_test)
