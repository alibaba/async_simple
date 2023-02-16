add_library(asio)
target_sources(asio
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    asio.cppm
)
target_include_directories(asio PUBLIC .)