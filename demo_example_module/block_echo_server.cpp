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

void session(tcp::socket sock) {
    int msg_index = 0;
    for (;;) {
        const size_t max_length = 1024;
        char data[max_length];
        auto [error, length] = read_some(sock, asio::buffer(data, max_length));
        msg_index++;
        if (error == asio::error::eof) {
            std::cout << "Remote client closed at message index: "
                      << msg_index - 1 << ".\n";
            break;
        } else if (error) {
            std::cout << error.message() << '\n';
            throw asio::system_error(error);
        }

        write(sock, asio::buffer(data, length));
    }
    std::cout << "Finished echo message, total: " << msg_index - 1 << ".\n";
}

void start_server(asio::io_context& io_context, unsigned short port) {
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "Listen port " << port << " successfully.\n";
    for (;;) {
        auto [error, socket] = accept(a);
        if (error) {
            std::cout << "Accept failed, error: " << error.message() << '\n';
            continue;
        }
        std::cout << "New client comming.\n";
        session(std::move(socket));

        std::error_code ec;
        socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        socket.close(ec);
    }
}

int main(int argc, char* argv[]) {
    try {
        asio::io_context io_context;
        start_server(io_context, 9980);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}