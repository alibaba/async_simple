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
#ifndef ASYNC_SIMPLE_DEMO_ASIO_UTIL_H
#define ASYNC_SIMPLE_DEMO_ASIO_UTIL_H
#include <asio.hpp>
template <typename AsioBuffer>
std::pair<asio::error_code, size_t> read_some(asio::ip::tcp::socket &sock,
                                              AsioBuffer &&buffer) {
    asio::error_code error;
    size_t length = sock.read_some(std::forward<AsioBuffer>(buffer), error);
    return std::make_pair(error, length);
}

template <typename AsioBuffer>
std::pair<asio::error_code, size_t> write(asio::ip::tcp::socket &sock,
                                          AsioBuffer &&buffer) {
    asio::error_code error;
    auto length = asio::write(sock, std::forward<AsioBuffer>(buffer), error);
    return std::make_pair(error, length);
}

std::pair<std::error_code, asio::ip::tcp::socket> accept(
    asio::ip::tcp::acceptor &a) {
    std::error_code error;
    auto socket = a.accept(error);
    return std::make_pair(error, std::move(socket));
}

std::pair<std::error_code, asio::ip::tcp::socket> connect(
    asio::io_context &io_context, std::string host, std::string port) {
    asio::ip::tcp::socket s(io_context);
    asio::ip::tcp::resolver resolver(io_context);
    std::error_code error;
    asio::connect(s, resolver.resolve(host, port), error);
    return std::make_pair(error, std::move(s));
}
#endif
