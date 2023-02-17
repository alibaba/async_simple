file(GLOB SRCS *.cppm)
file(GLOB CORO_SRCS coro/*.cppm)
file(GLOB EXEC_SRCS executors/*.cppm)
file(GLOB UTH_SRCS uthread/*.cppm)
file(GLOB UTH_IMPL_SRCS uthread/internal/*.cppm)
file(GLOB UTIL_SRCS util/*.cppm)
file(GLOB UTH_ASM_SRCS uthread/internal/*.S)
    
add_library(async_simple ${UTH_ASM_SRCS})
target_sources(async_simple
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${SRCS}
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${CORO_SRCS}
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${EXEC_SRCS}
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${UTH_SRCS}
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${UTH_IMPL_SRCS}
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${UTIL_SRCS}
)
target_link_libraries(async_simple PUBLIC std)

add_subdirectory(test)
