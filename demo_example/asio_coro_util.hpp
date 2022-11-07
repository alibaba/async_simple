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
#ifndef ASYNC_SIMPLE_DEMO_ASIO_CORO_UTIL_H
#define ASYNC_SIMPLE_DEMO_ASIO_CORO_UTIL_H
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"

#include <chrono>
#include "asio.hpp"

class AsioExecutor : public async_simple::Executor {
public:
    AsioExecutor(asio::io_context &io_context) : io_context_(io_context) {}

    virtual bool schedule(Func func) override {
        asio::post(io_context_, std::move(func));
        return true;
    }

private:
    asio::io_context &io_context_;
};

class AcceptorAwaiter {
public:
    AcceptorAwaiter(asio::ip::tcp::acceptor &acceptor,
                    asio::ip::tcp::socket &socket)
        : acceptor_(acceptor), socket_(socket) {}
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> handle) {
        acceptor_.async_accept(socket_, [this, handle](auto ec) mutable {
            ec_ = ec;
            handle.resume();
        });
    }
    auto await_resume() noexcept { return ec_; }
    auto coAwait(async_simple::Executor *executor) noexcept {
        return std::move(*this);
    }

private:
    asio::ip::tcp::acceptor &acceptor_;
    asio::ip::tcp::socket &socket_;
    std::error_code ec_{};
};

inline async_simple::coro::Lazy<std::error_code> async_accept(
    asio::ip::tcp::acceptor &acceptor, asio::ip::tcp::socket &socket) noexcept {
    co_return co_await AcceptorAwaiter{acceptor, socket};
}

template <typename Socket, typename AsioBuffer>
struct ReadSomeAwaiter {
public:
    ReadSomeAwaiter(Socket &socket, AsioBuffer &&buffer)
        : socket_(socket), buffer_(buffer) {}
    bool await_ready() { return false; }
    auto await_resume() { return std::make_pair(ec_, size_); }
    void await_suspend(std::coroutine_handle<> handle) {
        socket_.async_read_some(std::move(buffer_),
                                [this, handle](auto ec, auto size) mutable {
                                    ec_ = ec;
                                    size_ = size;
                                    handle.resume();
                                });
    }
    auto coAwait(async_simple::Executor *executor) noexcept {
        return std::move(*this);
    }

private:
    Socket &socket_;
    AsioBuffer buffer_;

    std::error_code ec_{};
    size_t size_{0};
};

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_read_some(Socket &socket, AsioBuffer &&buffer) noexcept {
    co_return co_await ReadSomeAwaiter{socket, std::move(buffer)};
}

template <typename Socket, typename AsioBuffer>
struct ReadAwaiter {
public:
    ReadAwaiter(Socket &socket, AsioBuffer &buffer)
        : socket_(socket), buffer_(buffer) {}
    bool await_ready() { return false; }
    auto await_resume() { return std::make_pair(ec_, size_); }
    void await_suspend(std::coroutine_handle<> handle) {
        asio::async_read(socket_, buffer_,
                         [this, handle](auto ec, auto size) mutable {
                             ec_ = ec;
                             size_ = size;
                             handle.resume();
                         });
    }
    auto coAwait(async_simple::Executor *executor) noexcept {
        return std::move(*this);
    }

private:
    Socket &socket_;
    AsioBuffer &buffer_;

    std::error_code ec_{};
    size_t size_{0};
};

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>> async_read(
    Socket &socket, AsioBuffer &buffer) noexcept {
    co_return co_await ReadAwaiter{socket, buffer};
}

template <typename Socket, typename AsioBuffer>
struct ReadUntilAwaiter {
public:
    ReadUntilAwaiter(Socket &socket, AsioBuffer &buffer,
                     asio::string_view delim)
        : socket_(socket), buffer_(buffer), delim_(delim) {}
    bool await_ready() { return false; }
    auto await_resume() { return std::make_pair(ec_, size_); }
    void await_suspend(std::coroutine_handle<> handle) {
        asio::async_read_until(socket_, buffer_, delim_,
                               [this, handle](auto ec, auto size) mutable {
                                   ec_ = ec;
                                   size_ = size;
                                   handle.resume();
                               });
    }
    auto coAwait(async_simple::Executor *executor) noexcept {
        return std::move(*this);
    }

private:
    Socket &socket_;
    AsioBuffer &buffer_;
    asio::string_view delim_;

    std::error_code ec_{};
    size_t size_{0};
};

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_read_until(Socket &socket, AsioBuffer &buffer,
                 asio::string_view delim) noexcept {
    co_return co_await ReadUntilAwaiter{socket, buffer, delim};
}

template <typename Socket, typename AsioBuffer>
struct WriteAwaiter {
public:
    WriteAwaiter(Socket &socket, AsioBuffer &&buffer)
        : socket_(socket), buffer_(std::move(buffer)) {}
    bool await_ready() { return false; }
    auto await_resume() { return std::make_pair(ec_, size_); }
    void await_suspend(std::coroutine_handle<> handle) {
        asio::async_write(socket_, std::move(buffer_),
                          [this, handle](auto ec, auto size) mutable {
                              ec_ = ec;
                              size_ = size;
                              handle.resume();
                          });
    }
    auto coAwait(async_simple::Executor *executor) noexcept {
        return std::move(*this);
    }

private:
    Socket &socket_;
    AsioBuffer buffer_;

    std::error_code ec_{};
    size_t size_{0};
};

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>> async_write(
    Socket &socket, AsioBuffer &&buffer) noexcept {
    co_return co_await WriteAwaiter{socket, std::move(buffer)};
}

class ConnectAwaiter {
public:
    ConnectAwaiter(asio::io_context &io_context, asio::ip::tcp::socket &socket,
                   const std::string &host, const std::string &port)
        : io_context_(io_context), socket_(socket), host_(host), port_(port) {}
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> handle) {
        asio::ip::tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, port_);
        asio::async_connect(socket_, endpoints,
                            [this, handle](std::error_code ec,
                                           asio::ip::tcp::endpoint) mutable {
                                ec_ = ec;
                                handle.resume();
                            });
    }
    auto await_resume() noexcept { return ec_; }
    auto coAwait(async_simple::Executor *executor) noexcept {
        return std::move(*this);
    }

private:
    asio::io_context &io_context_;
    asio::ip::tcp::socket &socket_;
    std::string host_;
    std::string port_;
    std::error_code ec_{};
};

inline async_simple::coro::Lazy<std::error_code> async_connect(
    asio::io_context &io_context, asio::ip::tcp::socket &socket,
    const std::string &host, const std::string &port) noexcept {
    co_return co_await ConnectAwaiter{io_context, socket, host, port};
}
#endif
