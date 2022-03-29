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
#include <iostream>
#include <thread>

#include "../../io_context_pool.hpp"
#include "connection.hpp"

using asio::ip::tcp;

class multilple_core_http_server {
public:
    multilple_core_http_server(io_context_pool& pool, unsigned short port)
        : pool_(pool), port_(port), executor_(pool.get_io_context()) {}

    async_simple::coro::Lazy<void> start() {
        tcp::acceptor a(pool_.get_io_context(),
                        tcp::endpoint(tcp::v4(), port_));
        for (;;) {
            tcp::socket socket(pool_.get_io_context());
            auto error = co_await async_accept(a, socket);
            if (error) {
                std::cout << "Accept failed, error: " << error.message()
                          << '\n';
                continue;
            }
            std::cout << "New client comming.\n";
            start_one(std::move(socket)).via(&executor_).start([](auto&&) {});
        }
    }

    async_simple::coro::Lazy<void> start_one(asio::ip::tcp::socket socket) {
        connection conn(std::move(socket), "./");
        co_await conn.start();
    }

private:
    io_context_pool& pool_;
    unsigned short port_;
    AsioExecutor executor_;
};

int main(int argc, char* argv[]) {
    try {
        io_context_pool pool(10);
        std::thread thd([&pool] { pool.run(); });
        multilple_core_http_server server(pool, 9980);
        async_simple::coro::syncAwait(server.start());
        thd.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}