if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin" AND
    NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

add_executable(CountChar CountChar.cpp)
target_link_libraries(CountChar async_simple)

if (${ASYNC_SIMPLE_BUILD_MODULES})
    add_executable(CountCharUsingModules CountCharUsingModules.cpp)
    target_link_libraries(CountCharUsingModules async_simple)
endif()

add_executable(ReadFiles ReadFiles.cpp)
target_link_libraries(ReadFiles async_simple)

if(NOT ASYNC_SIMPLE_DISABLE_AIO AND LIBAIO_INCLUDE_DIR AND LIBAIO_LIBRARIES)
  target_link_libraries(ReadFiles ${LIBAIO_LIBRARIES})
endif()

add_custom_command(
        TARGET ReadFiles POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/Input
                ${CMAKE_CURRENT_BINARY_DIR}/Input)

include_directories(asio)

add_executable(async_echo_server async_echo_server.cpp)
target_link_libraries(async_echo_server async_simple)

add_executable(async_echo_client async_echo_client.cpp)
target_link_libraries(async_echo_client async_simple)

add_executable(async_multiple_core_echo_server async_multiple_core_echo_server.cpp)
target_link_libraries(async_multiple_core_echo_server async_simple)

add_executable(block_echo_server block_echo_server.cpp)
target_link_libraries(block_echo_server ${CMAKE_THREAD_LIBS_INIT})

add_executable(block_echo_client block_echo_client.cpp)
target_link_libraries(block_echo_client ${CMAKE_THREAD_LIBS_INIT})

add_executable(http_server http/coroutine_http/http_server.cpp)
target_link_libraries(http_server async_simple)

add_executable(multiple_core_http_server http/coroutine_http/multiple_core_http_server.cpp)
target_link_libraries(multiple_core_http_server async_simple)

add_executable(http_client http/coroutine_http/http_client.cpp)
target_link_libraries(http_client async_simple)

add_executable(block_http_server http/block_http/block_http_server.cpp)
target_link_libraries(block_http_server async_simple)

add_executable(pmr_lazy pmr_lazy.cpp)
target_link_libraries(pmr_lazy async_simple)

SET(ENABLE_SSL OFF)

if (ENABLE_SSL)
    find_package(OpenSSL REQUIRED)
    add_definitions(-DENABLE_SSL)
    message(STATUS "Use SSL")
endif()

add_executable(smtp_client smtp/smtp_client.cpp)
if (ENABLE_SSL)
    target_link_libraries(smtp_client async_simple ${CMAKE_THREAD_LIBS_INIT} OpenSSL::SSL)
else()
    target_link_libraries(smtp_client async_simple ${CMAKE_THREAD_LIBS_INIT})
endif()
