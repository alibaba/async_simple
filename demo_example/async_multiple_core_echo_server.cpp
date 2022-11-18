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
#include <cstdlib>
#include <iostream>
#include <thread>
#include "asio_coro_util.hpp"
#include "io_context_pool.hpp"

using asio::ip::tcp;

async_simple::coro::Lazy<void> session(tcp::socket sock,
                                       std::shared_ptr<AsioExecutor>) {
    std::cout << "session thread id:" << std::this_thread::get_id()
              << std::endl;
    int msg_index = 0;
    for (;;) {
        constexpr size_t max_length = 1024;
        char data[max_length];
        auto [error, length] =
            co_await async_read_some(sock, asio::buffer(data, max_length));
        msg_index++;
        if (error == asio::error::eof) {
            std::cout << "Remote client closed at message index: "
                      << msg_index - 1 << ".\n";
            break;
        } else if (error) {
            std::cout << error.message() << '\n';
            break;
        }

        co_await async_write(sock, asio::buffer(data, length));
    }

    std::error_code ec;
    sock.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    sock.close(ec);

    std::cout << "Finished echo message, total: " << msg_index - 1 << ".\n";
}

int main(int argc, char* argv[]) {
    io_context_pool pool{std::thread::hardware_concurrency() * 2};
    auto thread = std::thread([&] { pool.run(); });
    async_simple::coro::syncAwait([&]() -> async_simple::coro::Lazy<void> {
        auto& io_context = pool.get_io_context();
        tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), 9980));
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            std::error_code ec;
            a.close(ec);
        });
        std::cout << "Listen port " << 9980 << " successfully.\n";
        for (;;) {
            auto& ctx = pool.get_io_context();
            tcp::socket socket(ctx);
            auto error = co_await async_accept(a, socket);
            if (error) {
                std::cout << "Accept failed, error: " << error.message()
                          << '\n';
                if (error == asio::error::operation_aborted)
                    break;
                continue;
            }
            std::cout << "New client coming.\n";
            auto executor = std::make_shared<AsioExecutor>(ctx);
            session(std::move(socket), executor).via(executor.get()).detach();
        }
    }());
    pool.stop();
    if (thread.joinable())
        thread.join();
    return 0;
}
