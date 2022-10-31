file(GLOB uthread_test_src "*.cpp")

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "13")
  # The address sanitizer may produce false-positive results for swap-context.
  # See https://github.com/google/sanitizers/issues/189 for details.
  # FIXME: A proper fix may add a __asan_enter_fiber and __asan_exit_fiber
  # for the user thread.
  # FIXME2: the new added sanitize action: address-use-after-return
  # can't handle stackful coroutine well too. So disable it as a workaround now.
  set_property(SOURCE ${uthread_test_src} PROPERTY COMPILE_OPTIONS "-fno-sanitize=address;-fsanitize-address-use-after-return=never")
endif()

add_executable(async_simple_uthread_test ${uthread_test_src} ${PROJECT_SOURCE_DIR}/async_simple/test/dotest.cpp)

target_link_libraries(async_simple_uthread_test async_simple ${deplibs} ${testdeplibs})

add_test(NAME run_async_simple_uthread_test COMMAND async_simple_uthread_test)

