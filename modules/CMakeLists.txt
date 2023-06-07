add_library(async_simple_modules)
target_sources(async_simple_modules
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    async_simple.cppm
)
target_include_directories(async_simple_modules PUBLIC ..)

if(NOT ${UTHREAD})
  add_definitions(-DASYNC_SIMPLE_MODULES_HAS_NOT_UTHREAD)
  # FIXME: It looks like `target_compile_definitions` doesn't work
  # for CXX_MODULES FILES. We should file an issue for this.
  # target_compile_definitions(async_simple_modules INTERFACE ASYNC_SIMPLE_MODULES_HAS_NOT_UTHREAD)
endif()

if (${ASYNC_SIMPLE_DISABLE_AIO} OR NOT LIBAIO_INCLUDE_DIR OR NOT LIBAIO_LIBRARIES)
  target_compile_definitions(libasync_simple INTERFACE ASYNC_SIMPLE_HAS_NOT_AIO)
endif()
