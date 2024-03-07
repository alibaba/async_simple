add_executable(CountChar CountChar.cpp)
target_link_libraries(CountChar async_simple std pthread)

add_executable(ReadFiles ReadFiles.cpp)
target_link_libraries(ReadFiles async_simple std pthread)

add_custom_command(
        TARGET ReadFiles POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/demo_example_module/Input
                ${CMAKE_CURRENT_BINARY_DIR}/Input)

add_library(asio_util)
target_sources(asio_util
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    asio_util.cppm
)
target_link_libraries(asio_util PUBLIC std async_simple asio)

add_executable(async_echo_server async_echo_server.cpp)
target_link_libraries(async_echo_server async_simple std asio_util pthread)

add_executable(async_echo_client async_echo_client.cpp)
target_link_libraries(async_echo_client async_simple std asio_util pthread)

add_executable(block_echo_server block_echo_server.cpp)
target_link_libraries(block_echo_server pthread std asio_util pthread)

add_executable(block_echo_client block_echo_client.cpp)
target_link_libraries(block_echo_client pthread std asio_util pthread)

add_executable(http_server http/coroutine_http/http_server.cpp)
target_link_libraries(http_server async_simple std asio_util pthread)

add_executable(http_client http/coroutine_http/http_client.cpp)
target_link_libraries(http_client async_simple std asio_util pthread)

add_executable(block_http_server http/block_http/block_http_server.cpp)
target_link_libraries(block_http_server async_simple std asio_util pthread)
