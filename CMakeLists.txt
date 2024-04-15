cmake_minimum_required(VERSION 3.28)
project(async_simple CXX C ASM)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

find_package(GTest REQUIRED)

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
    -Wno-deprecated-declarations
    -D__STDC_LIMIT_MACROS
    -g
    )

add_link_options(
    -fuse-ld=lld 
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.0")
    set(CXX_FLAGS "${CXX_FLAGS} -fexperimental-modules-reduced-bmi")
endif()

set(HEADERS_PATH
    ${PROJECT_SOURCE_DIR}
    ${GTEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    )
include_directories(${HEADERS_PATH})

set(deplibs)
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

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

add_subdirectory(third_party_module/asio)
add_subdirectory(third_party_module/stdmodules)
add_subdirectory(async_simple_module)
add_subdirectory(demo_example_module)
