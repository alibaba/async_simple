cmake_minimum_required(VERSION 3.15)
project(async_simple
        VERSION 1.2
        LANGUAGES CXX C ASM)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(libasync_simple INTERFACE)
target_compile_features(libasync_simple INTERFACE cxx_std_20)
target_include_directories(libasync_simple INTERFACE
        $<BUILD_INTERFACE:${async_simple_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        )

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

find_package(Threads REQUIRED)
target_link_libraries(libasync_simple INTERFACE Threads::Threads)
find_package(Aio QUIET)

find_package(Benchmark)

option(ASYNC_SIMPLE_ENABLE_TESTS "Build the tests" ON)
option(ASYNC_SIMPLE_DISABLE_AIO "Don't build SimpleIOExecutor with AIO" OFF)

if(NOT ${ASYNC_SIMPLE_DISABLE_AIO} AND LIBAIO_INCLUDE_DIR AND LIBAIO_LIBRARIES)
    message(STATUS "aio found")
    message(STATUS "aio: ${LIBAIO_INCLUDE_DIR}, ${LIBAIO_LIBRARIES}.")
else()
    # Just for SimpleIOExecutor.h
    add_definitions(-DASYNC_SIMPLE_HAS_NOT_AIO)
endif()

if (${ASYNC_SIMPLE_ENABLE_TESTS})
    find_package(GTest)
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
        /Zc:__cplusplus
        /await:strict
        /EHa
        )
else()
    set(CXX_FLAGS
        -std=c++20
        -D_GLIBCXX_USE_CXX11_ABI=1
        -Wno-deprecated-register
        -Wno-mismatched-new-delete
        -D_FILE_OFFSET_BITS=64
        -fPIC
        -Wall
        -Werror
        -D__STDC_LIMIT_MACROS
        -g
        )
endif()

set(IS_ACC OFF CACHE INTERNAL "Whether the current compiler is ACC")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
try_compile(IS_ACC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/cmake/IsACC.cpp)
if (${IS_ACC})
    message("-- Using ACC")
    # It is super rare that the destructor of an exception can be throwable.
    # But such little probability prevents optimizations for coroutines
    # significantly.
    # Since the the body of coroutines are wrapped into a big try-catch.
    # So the coroutines can be marked as noexcept automatically if all its
    # promise functions and the allocation functions matches the behavior.
    list(APPEND CXX_FLAGS "-fdisable-exception-with-may-throw-dtor")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND CXX_FLAGS "-fcoroutines")
endif()

set(HEADERS_PATH
    ${GTEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    ${AIO_INCLUDE_DIR}
    )
include_directories(${HEADERS_PATH})
set(testdeplibs)
list(APPEND testdeplibs ${GTEST_LIBRARIES})
list(APPEND testdeplibs ${GMOCK_LIBRARIES})
list(APPEND testdeplibs ${CMAKE_THREAD_LIBS_INIT})
if(NOT ${ASYNC_SIMPLE_DISABLE_AIO} AND LIBAIO_INCLUDE_DIR AND LIBAIO_LIBRARIES)
    list(APPEND testdeplibs ${LIBAIO_LIBRARIES})
endif()

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

# Start Detecing Uthread
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux") # uname -s
  if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64|aarch64|ppc64le") # uname -m
    set(UTHREAD ON)
    message("-- Uthread on")
  endif()
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin") # uname -s
  if ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64|arm64") # uname -m
    set(UTHREAD ON)
    message("-- Uthread on")
  endif()
endif()
# End Detecing Uthread


string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

add_subdirectory(async_simple)

option(ASYNC_SIMPLE_BUILD_MODULES "Build async_simple library in C++20 Modules form" OFF)
if(${ASYNC_SIMPLE_BUILD_MODULES}) 
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        include(cxx_modules_rules_clang.cmake)
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
    endif()

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    add_subdirectory(modules)
endif()

option(ASYNC_SIMPLE_BUILD_DEMO_EXAMPLE "Build the demo example" ON)
if (ASYNC_SIMPLE_BUILD_DEMO_EXAMPLE)
    add_subdirectory(demo_example)
endif ()

if (BENCHMARK_INCLUDE_DIR AND BENCHMARK_LIBRARIES)
    message(STATUS "Building benchmarks")
    add_subdirectory(benchmarks)
else()
    message(STATUS "Skipping benchmarks")
endif()
