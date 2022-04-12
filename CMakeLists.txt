cmake_minimum_required(VERSION 3.12)
project(async_simple CXX C ASM)
enable_testing()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

find_package(Threads)
find_package(Aio)

find_package(Benchmark)

option(ASYNC_SIMPLE_ENABLE_TESTS "Build the tests" ON)

if(LIBAIO_INCLUDE_DIR AND LIBAIO_LIBRARIES)
    message(STATUS "aio found")
    message(STATUS "aio: ${LIBAIO_INCLUDE_DIR}, ${LIBAIO_LIBRARIES}.")
else()
    add_compile_definitions(ASYNC_SIMPLE_HAS_NOT_AIO)
endif()

if (${ASYNC_SIMPLE_ENABLE_TESTS})
    find_package(GMock REQUIRED)
    find_package(GTest REQUIRED)

    if(GTEST_INCLUDE_DIR AND GTEST_LIBRARIES)
        message(STATUS "GTest found.")
        message(STATUS "GTest: ${GTEST_INCLUDE_DIR}, ${GTEST_LIBRARIES}")
    else()
        message(FATAL_ERROR "GTest NOT found. Set ASYNC_SIMPLE_ENABLE_TESTS OFF to skip tests.")
    endif()

    if(GMOCK_INCLUDE_DIR AND GMOCK_LIBRARIES)
        message(STATUS "GMock found.")
        message(STATUS "GMock: ${GMOCK_INCLUDE_DIR}, ${GMOCK_LIBRARIES}")
    else()
        message(FATAL_ERROR "GMock NOT found. Set ASYNC_SIMPLE_ENABLE_TESTS OFF to skip tests.")
    endif()
endif()

if (BENCHMARK_INCLUDE_DIR AND BENCHMARK_LIBRARIES)
    message(STATUS "Benchmark found.")
    message(STATUS "Benchmark: ${BENCHMARK_INCLUDE_DIR}, ${BENCHMARK_LIBRARIES}")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# set CXXFALGS
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CXX_FLAGS
        /std:c++20
        /await:strict
        /EHa
        )
else()
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
endif()

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
list(APPEND deplibs ${CMAKE_THREAD_LIBS_INIT})
if(LIBAIO_LIBRARIES)
    list(APPEND deplibs ${LIBAIO_LIBRARIES})
endif()

set(testdeplibs)
list(APPEND testdeplibs ${GTEST_LIBRARIES})
list(APPEND testdeplibs ${GMOCK_LIBRARIES})
list(APPEND testdeplibs ${CMAKE_THREAD_LIBS_INIT})

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
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
endif()

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("-- Use flag -fsanitize=address")
        list(APPEND CXX_FLAGS "-fsanitize=address")
    endif()
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux") # uname -s
  if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64|aarch64") # uname -p
    set(UTHREAD ON)
    message("-- Uthread on")
  endif()
endif()

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

add_subdirectory(async_simple)
add_subdirectory(demo_example)
if (BENCHMARK_INCLUDE_DIR AND BENCHMARK_LIBRARIES)
    message(STATUS "Building benchmarks")
    add_subdirectory(benchmarks)
else()
    message(STATUS "Skipping benchmarks")
endif()
