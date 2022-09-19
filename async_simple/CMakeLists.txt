file(GLOB coro_src "coro/*.cpp")
file(GLOB executors_src "executors/*.cpp")

if(UTHREAD)
  file(GLOB uthread_src "uthread/internal/*.cc")
  # Compiler-rt15 enables new sanitizer option sanitize-address-use-after-return,
  # But it can't take user thread well. So disable it for thread.cc as a workaround.
  if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
      CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "13")
    set_property(SOURCE ${uthread_src} PROPERTY COMPILE_FLAGS -fsanitize-address-use-after-return=never)
  endif()
  if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
    file(GLOB uthread_asm_src "uthread/internal/*arm64_aapcs_elf*")
  elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    file(GLOB uthread_asm_src "uthread/internal/*x86_64_sysv_elf*")
  endif()
endif()

file(GLOB headers "*.h")
file(GLOB coro_header "coro/*.h")
file(GLOB executors_header "executors/*.h")
file(GLOB experimental_header "experimental/*.h")
file(GLOB util_header "util/*.h")
if(UTHREAD)
  file(GLOB uthread_header "uthread/*.h")
  file(GLOB uthread_internal_header "uthread/internal/*.h")
endif()


set(SRCS
  ${coro_src}
  ${executors_src}
  )
if(UTHREAD)
  list(APPEND SRCS ${uthread_src})
  list(APPEND SRCS ${uthread_asm_src})
endif()

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_library(async_simple_static STATIC ${SRCS})
  add_library(async_simple SHARED ${SRCS})
  target_link_libraries(async_simple PUBLIC libasync_simple)
  target_link_libraries(async_simple_static PUBLIC libasync_simple)

  set_target_properties(async_simple_static PROPERTIES OUTPUT_NAME "async_simple")

  install(TARGETS async_simple DESTINATION lib/)
  install(TARGETS async_simple_static DESTINATION lib/)
else()
  add_library(async_simple STATIC ${SRCS})
  target_link_libraries(async_simple PUBLIC libasync_simple)
  install(TARGETS async_simple DESTINATION lib/)
endif()

install(FILES ${headers} DESTINATION include/async_simple)
install(FILES ${coro_header} DESTINATION include/async_simple/coro)
install(FILES ${executors_header} DESTINATION include/async_simple/executors)
install(FILES ${experimental_header} DESTINATION include/async_simple/experimental)
install(FILES ${util_header} DESTINATION include/async_simple/util)
if(UTHREAD)
  install(FILES ${uthread_header} DESTINATION include/async_simple/uthread)
  install(FILES ${uthread_internal_header} DESTINATION include/async_simple/uthread/internal)
endif()

if (${ASYNC_SIMPLE_ENABLE_TESTS})
  add_subdirectory(test)
  add_subdirectory(util/test)
  add_subdirectory(coro/test)
  add_subdirectory(executors/test)
  if(UTHREAD)
    add_subdirectory(uthread/test)
  endif()
endif()
if (NOT TARGET async_simple::async_simple_header_only)
  add_library(async_simple::async_simple_header_only ALIAS libasync_simple)
endif ()
