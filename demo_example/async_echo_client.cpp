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
#include <cstring>
#include <iostream>
#include "asio_util.hpp"
#include <async_simple/coro/SpinLock.h>

static int concurrency = 4;
static async_simple::coro::SpinLock g_wlock, g_rlock;

using asio::ip::tcp;

async_simple::coro::Lazy<int> task(asio::ip::tcp::socket& socket) {
    const int max_length = 1024;
    char write_buf[max_length] = {"hello async_simple"};
    char read_buf[max_length];
    auto send = [&]() -> async_simple::coro::Lazy<> {
        auto scopeLock = co_await g_wlock.coScopedLock();
        co_await async_write(socket, asio::buffer(write_buf, max_length));
        co_return;
    };
    auto recv = [&]() -> async_simple::coro::Lazy<> {
        auto scopeLock = co_await g_rlock.coScopedLock();
        auto [error, reply_length] = co_await async_read_some(
            socket, asio::buffer(read_buf, max_length));
        if (error) {
            std::cout << "recv error: " << error.message() << std::endl;
            throw asio::system_error(error);
        }
    };

    while (true) {
        co_await send();
        co_await recv();
    }
}

async_simple::coro::Lazy<void> start(asio::io_context &io_context,
                                     std::string host, std::string port) {
    asio::ip::tcp::socket socket(io_context);
    auto ec = co_await async_connect(io_context, socket, host, port);
    if (ec) {
        std::cout << "Connect error: " << ec.message() << '\n';
        throw asio::system_error(ec);
    }
    std::cout << "Connect to " << host << ":" << port << " successfully.\n";

    std::vector<async_simple::coro::Lazy<int>> input;
    for (int i = 0; i < concurrency; ++i) {
        input.push_back(task(socket));
    }
    co_await async_simple::coro::collectAll(std::move(input));

    // const int max_length = 1024;
    // char write_buf[max_length] = {"hello async_simple"};
    // char read_buf[max_length];
    // const int count = 100000;
    // for (int i = 0; i < count; ++i) {
    //     co_await async_write(socket, asio::buffer(write_buf, max_length));
    //     auto [error, reply_length] = co_await async_read_some(
    //         socket, asio::buffer(read_buf, max_length));
    //     if (error == asio::error::eof) {
    //         std::cout << "eof at message index: " << i << '\n';
    //         break;
    //     } else if (error) {
    //         std::cout << "error: " << error.message()
    //                   << ", message index: " << i << '\n';
    //         throw asio::system_error(error);
    //     }
    //
    //     // handle read data as your wish.
    // }
    //
    // std::cout << "Finished send and recieve " << count
    //           << " messages, client will close.\n";
    std::error_code ignore_ec;
    socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
    io_context.stop();
}

int main(int argc, char *argv[]) {
    try {
        asio::io_context io_context;
        std::thread thd([&io_context] {
            asio::io_context::work work(io_context);
            io_context.run();
        });
        async_simple::executors::SimpleExecutor e(10);
        async_simple::coro::syncAwait(start(io_context, "172.16.136.243", "9980").via(&e));
        io_context.stop();
        thd.join();
        std::cout << "Finished ok, client quit.\n";
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}