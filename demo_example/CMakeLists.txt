add_executable(CountChar CountChar.cpp)
target_link_libraries(CountChar async_simple)

add_executable(ReadFiles ReadFiles.cpp)
target_link_libraries(ReadFiles async_simple)

add_custom_command(
        TARGET ReadFiles POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/demo_example/Input
                ${CMAKE_CURRENT_BINARY_DIR}/Input)

include_directories(asio)

add_executable(async_echo_server async_echo_server.cpp)
target_link_libraries(async_echo_server async_simple)

add_executable(async_echo_client async_echo_client.cpp)
target_link_libraries(async_echo_client async_simple)

add_executable(block_echo_server block_echo_server.cpp)
target_link_libraries(block_echo_server pthread)

add_executable(block_echo_client block_echo_client.cpp)
target_link_libraries(block_echo_client pthread)

add_executable(http_server http/coroutine_http/http_server.cpp)
target_link_libraries(http_server async_simple)

add_executable(http_client http/coroutine_http/http_client.cpp)
target_link_libraries(http_client async_simple)

add_executable(block_http_server http/block_http/block_http_server.cpp)
target_link_libraries(block_http_server async_simple)

SET(ENABLE_SSL OFF)

if (ENABLE_SSL)
    find_package(OpenSSL REQUIRED)
    add_definitions(-DENABLE_SSL)
    message(STATUS "Use SSL")
endif()

add_executable(smtp_client smtp/smtp_client.cpp)
if (ENABLE_SSL)
    target_link_libraries(smtp_client stdc++fs async_simple OpenSSL::SSL)
else()
    target_link_libraries(smtp_client stdc++fs async_simple)
endif()
