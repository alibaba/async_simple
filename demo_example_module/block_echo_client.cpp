/*
 * Copyright (c) 2022, Alibaba Group Holding Limited;
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import std;
import asio_util;

using asio::ip::tcp;

void start(asio::io_context& io_context, std::string host, std::string port) {
    auto [ec, socket] = connect(io_context, host, port);
    if (ec) {
        std::cout << "Connect error: " << ec.message() << '\n';
        throw asio::system_error(ec);
    }
    std::cout << "Connect to " << host << ":" << port << " successfully.\n";
    const int max_length = 1024;
    char write_buf[max_length] = {"hello async_simple"};
    char read_buf[max_length];
    const int count = 10000;
    for (int i = 0; i < count; ++i) {
        write(socket, asio::buffer(write_buf, max_length));
        auto [error, reply_length] =
            read_some(socket, asio::buffer(read_buf, max_length));
        if (error == asio::error::eof) {
            std::cout << "eof at message index: " << i << '\n';
            break;
        } else if (error) {
            std::cout << "error: " << error.message()
                      << ", message index: " << i << '\n';
            throw asio::system_error(error);
        }

        // handle read data as your wish.
    }

    std::cout << "Finished send and recieve " << count
              << " messages, client will close.\n";
    std::error_code ignore_ec;
    socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
    socket.close(ignore_ec);
}

int main(int argc, char* argv[]) {
    try {
        asio::io_context io_context;
        start(io_context, "127.0.0.1", "9980");
        std::cout << "Finished ok, client quit.\n";
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}