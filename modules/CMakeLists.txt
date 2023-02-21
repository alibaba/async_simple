add_library(async_simple_modules)
target_sources(async_simple_modules
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    async_simple.cppm
)
target_include_directories(async_simple_modules PUBLIC ..)
