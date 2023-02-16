file(GLOB SRCS *.cppm)

add_library(std)
target_sources(std
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${SRCS}
)
