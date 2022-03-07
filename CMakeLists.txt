cmake_minimum_required(VERSION 3.8)
project(async_simple CXX C ASM)
enable_testing()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

find_package(Aio REQUIRED)
find_package(GMock REQUIRED)
find_package(GTest REQUIRED)
find_package(Benchmark)

if(LIBAIO_INCLUDE_DIR AND LIBAIO_LIBRARIES)
    message(STATUS "aio found")
    message(STATUS "aio: ${LIBAIO_INCLUDE_DIR}, ${LIBAIO_LIBRARIES}.")
else()
    message(FATAL_ERROR "aio NOT found")
endif()

if(GTEST_INCLUDE_DIR AND GTEST_LIBRARIES)
    message(STATUS "GTest found.")
    message(STATUS "GTest: ${GTEST_INCLUDE_DIR}, ${GTEST_LIBRARIES}")
else()
    message(FATAL_ERROR "GTest NOT found.")
endif()

if(GMOCK_INCLUDE_DIR AND GMOCK_LIBRARIES)
    message(STATUS "GMock found.")
    message(STATUS "GMock: ${GMOCK_INCLUDE_DIR}, ${GMOCK_LIBRARIES}")
else()
    message(FATAL_ERROR "GMock NOT found.")
endif()

if (BENCHMARK_INCLUDE_DIR AND BENCHMARK_LIBRARIES)
    message(STATUS "Benchmark found.")
    message(STATUS "Benchmark: ${BENCHMARK_INCLUDE_DIR}, ${BENCHMARK_LIBRARIES}")
endif()


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# set CXXFALGS
set(CXX_FLAGS
    -std=c++20
    -D_GLIBCXX_USE_CXX11_ABI=1
    -Wno-deprecated-register
    -D_FILE_OFFSET_BITS=64
    -fPIC
    -Wall
    -Werror
    -D__STDC_LIMIT_MACROS
    -g
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND CXX_FLAGS "-fcoroutines")
endif()

set(HEADERS_PATH
    ${PROJECT_SOURCE_DIR}
    ${GTEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    ${AIO_INCLUDE_DIR}
    )
include_directories(${HEADERS_PATH})

set(deplibs)
list(APPEND deplibs ${LIBAIO_LIBRARIES})
list(APPEND deplibs pthread)
set(testdeplibs)
list(APPEND testdeplibs ${GTEST_LIBRARIES})
list(APPEND testdeplibs ${GMOCK_LIBRARIES})

if(CMAKE_BUILD_BITS EQUAL 32)
    message("-- Use flag -m32")
    list(APPEND CXX_FLAGS "-m32")
    list(APPEND CXX_FLAGS "-DTARGET_32")
    list(APPEND deplibs "-m32")
else()
    message("-- Use flag -m64")
    list(APPEND CXX_FLAGS "-m64")
    list(APPEND CXX_FLAGS "-DTARGET_64")
    #list(APPEND deplibs "-m64")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("-- Use flag -fsanitize=address")
    list(APPEND CXX_FLAGS "-fsanitize=address")
endif()

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

add_subdirectory(async_simple)
add_subdirectory(demo_example)
if (BENCHMARK_INCLUDE_DIR AND BENCHMARK_LIBRARIES)
    message(STATUS "Building benchmarks")
    add_subdirectory(benchmarks)
else()
    message(STATUS "Skipping benchmarks")
endif()
