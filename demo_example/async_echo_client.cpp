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
#include <chrono>
#include "asio_util.hpp"
#include <async_simple/coro/SpinLock.h>
#include <async_simple/coro/Sleep.h>

using namespace std::chrono_literals;

static int concurrency = 4;
static async_simple::coro::SpinLock g_wlock, g_rlock;
static uint64_t count = 0;

async_simple::coro::Lazy<> show_qps() {
    while (true) {
        co_await async_simple::coro::sleep(1s);
        std::cout << "qps: " << count << std::endl;
        count = 0;
    }
}

async_simple::coro::Lazy<> task(asio::ip::tcp::socket& socket) {
    const int max_length = 4096;
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
        count++;
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

    std::vector<async_simple::coro::Lazy<>> input;
    for (int i = 0; i < concurrency; ++i) {
        input.push_back(task(socket));
    }
    input.push_back(show_qps());
    co_await async_simple::coro::collectAll(std::move(input));

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
        async_simple::executors::SimpleExecutor e(16);
        async_simple::coro::syncAwait(start(io_context, /*172.16.136.243*/"127.0.0.1", "9980").via(&e));
        io_context.stop();
        thd.join();
        std::cout << "Finished ok, client quit.\n";
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}