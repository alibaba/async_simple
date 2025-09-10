if (DEFINED GMOCK_INCLUDE_DIR AND DEFINED GTEST_INCLUDE_DIR
    AND DEFINED GTEST_LIBRARIES AND DEFINED GMOCK_LIBRARIES)
    message(STATUS "Using GTest from specified path")
else()
    message(STATUS "Not all GTest variable got specified: "
                   "'GMOCK_INCLUDE_DIR', 'GTEST_INCLUDE_DIR', "
                   "'GTEST_LIBRARIES', 'GMOCK_LIBRARIES'")
    message(STATUS "fetch GTest from https://github.com/google/googletest.git")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0
        GIT_SHALLOW ON
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
       CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "20")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=character-conversion")
    endif()
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(BUILD_GTEST ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    set(GMOCK_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include/)
    set(GTEST_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include/)
    set(GTEST_LIBRARIES gtest_main)
    set(GMOCK_LIBRARIES gmock_main)
endif()

message(STATUS "GTest: ${GTEST_INCLUDE_DIR}, ${GTEST_LIBRARIES}")
message(STATUS "GMock: ${GMOCK_INCLUDE_DIR}, ${GMOCK_LIBRARIES}")
