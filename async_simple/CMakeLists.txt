if(${UTHREAD})
  file(GLOB uthread_src "uthread/internal/*.cc")
  if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
      CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "13")
  endif()
file(GLOB uthread_asm_src "uthread/internal/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/*.S")
endif()

file(GLOB headers "*.h")
file(GLOB coro_header "coro/*.h")
file(GLOB experimental_header "experimental/*.h")
file(GLOB util_header "util/*.h")
if(UTHREAD)
  file(GLOB uthread_header "uthread/*.h")
  file(GLOB uthread_internal_header "uthread/internal/*.h")
endif()


set(SRCS "")
if(UTHREAD)
  list(APPEND SRCS ${uthread_src})
  list(APPEND SRCS ${uthread_asm_src})
endif()


if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0.0")
    target_compile_options(async_simple_header_only INTERFACE "-fcoroutines")
  endif()
endif()
# If there is no Uthread, async_simple is a header only library 
if(UTHREAD)
  add_library(async_simple_static STATIC ${SRCS})
  add_library(async_simple SHARED ${SRCS})
  target_link_libraries(async_simple PUBLIC async_simple_header_only)
  target_link_libraries(async_simple_static PUBLIC async_simple_header_only)
  install(TARGETS async_simple EXPORT async_simple_targets DESTINATION lib/)
  install(TARGETS async_simple_static EXPORT async_simple_targets DESTINATION lib/)
else()
  add_library(async_simple_static INTERFACE)
  target_link_libraries(async_simple_static INTERFACE async_simple_header_only)
  install(TARGETS async_simple_static EXPORT async_simple_targets DESTINATION lib/)
  add_library(async_simple INTERFACE)
  target_link_libraries(async_simple INTERFACE async_simple_header_only)
  install(TARGETS async_simple EXPORT async_simple_targets DESTINATION lib/)
endif()

install(TARGETS async_simple_header_only EXPORT async_simple_targets)

set_target_properties(async_simple PROPERTIES 
                                   VERSION ${PROJECT_VERSION}
                                   SOVERSION ${PROJECT_VERSION_MAJOR})

install(FILES ${headers} DESTINATION include/async_simple)
install(FILES ${coro_header} DESTINATION include/async_simple/coro)
install(FILES ${executors_header} DESTINATION include/async_simple/executors)
install(FILES ${experimental_header} DESTINATION include/async_simple/experimental)
install(FILES ${util_header} DESTINATION include/async_simple/util)
if(UTHREAD)
  install(FILES ${uthread_header} DESTINATION include/async_simple/uthread)
  install(FILES ${uthread_internal_header} DESTINATION include/async_simple/uthread/internal)
endif()

install(EXPORT async_simple_targets
        FILE async_simple-targets.cmake
        NAMESPACE async_simple::
        DESTINATION share/async_simple)

# Generate the config file in the current binary dir (this ensures it's not placed directly in source)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/async_simple-config.cmake"
"include(CMakeFindDependencyMacro)\n"
"find_package(Threads REQUIRED)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/async_simple-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/async_simple-config.cmake"
        DESTINATION share/async_simple)


if (${ASYNC_SIMPLE_ENABLE_TESTS})
  add_subdirectory(test)
  add_subdirectory(util/test)
  add_subdirectory(coro/test)
  add_subdirectory(executors/test)
  if(UTHREAD)
    add_subdirectory(uthread/test)
  endif()
endif()
