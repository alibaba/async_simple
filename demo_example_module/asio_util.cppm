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
export module asio_util;
import std;
export import async_simple;
export import experimental.coroutine;
export import asio;
export template <typename AsioBuffer>
std::pair<asio::error_code, size_t> read_some(asio::ip::tcp::socket& sock,
                                              AsioBuffer&& buffer) {
    asio::error_code error;
    size_t length = sock.read_some(std::forward<AsioBuffer>(buffer), error);
    return std::make_pair(error, length);
}

export template <typename AsioBuffer>
std::pair<asio::error_code, size_t> write(asio::ip::tcp::socket& sock,
                                          AsioBuffer&& buffer) {
    asio::error_code error;
    auto length = asio::write(sock, std::forward<AsioBuffer>(buffer), error);
    return std::make_pair(error, length);
}

export std::pair<std::error_code, asio::ip::tcp::socket> accept(
    asio::ip::tcp::acceptor& a) {
    std::error_code error;
    auto socket = a.accept(error);
    return std::make_pair(error, std::move(socket));
}

export std::pair<std::error_code, asio::ip::tcp::socket> connect(
    asio::io_context& io_context, std::string host, std::string port) {
    asio::ip::tcp::socket s(io_context);
    asio::ip::tcp::resolver resolver(io_context);
    std::error_code error;
    asio::connect(s, resolver.resolve(host, port), error);
    return std::make_pair(error, std::move(s));
}

export class AsioExecutor : public async_simple::Executor {
public:
    AsioExecutor(asio::io_context& io_context) : io_context_(io_context) {}
    using async_simple::Executor::Context;
    using async_simple::Executor::Func;

    virtual bool schedule(Func func) override {
        asio::post(io_context_, std::move(func));
        return true;
    }

    virtual bool currentThreadInExecutor() const override { return false; }

    async_simple::ExecutorStat stat() const override {
        return async_simple::ExecutorStat();
    }

    virtual async_simple::IOExecutor* getIOExecutor() override {
        return nullptr;
    }

private:
    asio::io_context& io_context_;
};

export class AcceptorAwaiter {
public:
    AcceptorAwaiter(asio::ip::tcp::acceptor& acceptor,
                    asio::ip::tcp::socket socket)
        : acceptor_(acceptor), socket_(std::move(socket)) {}
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::experimental::coroutine_handle<> handle) {
        acceptor_.async_accept([this, handle](auto ec, auto socket) mutable {
            ec_ = ec;
            socket_ = std::move(socket);
            handle.resume();
        });
    }
    auto await_resume() noexcept {
        return std::make_pair(ec_, std::move(socket_));
    }
    auto coAwait(async_simple::Executor* executor) noexcept {
        return std::move(*this);
    }

private:
    asio::ip::tcp::acceptor& acceptor_;
    asio::ip::tcp::socket socket_;
    std::error_code ec_{};
};

export inline async_simple::coro::Lazy<
    std::pair<std::error_code, asio::ip::tcp::socket>>
async_accept(asio::ip::tcp::acceptor& acceptor) noexcept {
    co_return co_await AcceptorAwaiter{
        acceptor, asio::ip::tcp::socket(acceptor.get_executor())};
}

export template <typename AsioBuffer>
struct ReadSomeAwaiter {
public:
    ReadSomeAwaiter(asio::ip::tcp::socket& socket, AsioBuffer&& buffer)
        : socket_(socket), buffer_(buffer) {}
    bool await_ready() { return false; }
    auto await_resume() { return std::make_pair(ec_, size_); }
    void await_suspend(std::experimental::coroutine_handle<> handle) {
        socket_.async_read_some(std::move(buffer_),
                                [this, handle](auto ec, auto size) mutable {
                                    ec_ = ec;
                                    size_ = size;
                                    handle.resume();
                                });
    }
    auto coAwait(async_simple::Executor* executor) noexcept {
        return std::move(*this);
    }

private:
    asio::ip::tcp::socket& socket_;
    AsioBuffer buffer_;

    std::error_code ec_{};
    size_t size_{0};
};

export template <typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_read_some(asio::ip::tcp::socket& socket, AsioBuffer&& buffer) noexcept {
    co_return co_await ReadSomeAwaiter{socket, std::move(buffer)};
}

export template <typename AsioBuffer>
struct ReadAwaiter {
public:
    ReadAwaiter(asio::ip::tcp::socket& socket, AsioBuffer& buffer)
        : socket_(socket), buffer_(buffer) {}
    bool await_ready() { return false; }
    auto await_resume() { return std::make_pair(ec_, size_); }
    void await_suspend(std::experimental::coroutine_handle<> handle) {
        asio::async_read(socket_, buffer_,
                         [this, handle](auto ec, auto size) mutable {
                             ec_ = ec;
                             size_ = size;
                             handle.resume();
                         });
    }
    auto coAwait(async_simple::Executor* executor) noexcept {
        return std::move(*this);
    }

private:
    asio::ip::tcp::socket& socket_;
    AsioBuffer& buffer_;

    std::error_code ec_{};
    size_t size_{0};
};

export template <typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>> async_read(
    asio::ip::tcp::socket& socket, AsioBuffer& buffer) noexcept {
    co_return co_await ReadAwaiter{socket, buffer};
}

export template <typename AsioBuffer>
struct ReadUntilAwaiter {
public:
    ReadUntilAwaiter(asio::ip::tcp::socket& socket, AsioBuffer& buffer,
                     std::string_view delim)
        : socket_(socket), buffer_(buffer), delim_(delim) {}
    bool await_ready() { return false; }
    auto await_resume() { return std::make_pair(ec_, size_); }
    void await_suspend(std::experimental::coroutine_handle<> handle) {
        asio::async_read_until(socket_, buffer_, delim_,
                               [this, handle](auto ec, auto size) mutable {
                                   ec_ = ec;
                                   size_ = size;
                                   handle.resume();
                               });
    }
    auto coAwait(async_simple::Executor* executor) noexcept {
        return std::move(*this);
    }

private:
    asio::ip::tcp::socket& socket_;
    AsioBuffer& buffer_;
    std::string_view delim_;

    std::error_code ec_{};
    size_t size_{0};
};

export template <typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_read_until(asio::ip::tcp::socket& socket, AsioBuffer& buffer,
                 std::string_view delim) noexcept {
    co_return co_await ReadUntilAwaiter{socket, buffer, delim};
}

export template <typename AsioBuffer>
struct WriteAwaiter {
public:
    WriteAwaiter(asio::ip::tcp::socket& socket, AsioBuffer&& buffer)
        : socket_(socket), buffer_(std::move(buffer)) {}
    bool await_ready() { return false; }
    auto await_resume() { return std::make_pair(ec_, size_); }
    void await_suspend(std::experimental::coroutine_handle<> handle) {
        asio::async_write(socket_, std::move(buffer_),
                          [this, handle](auto ec, auto size) mutable {
                              ec_ = ec;
                              size_ = size;
                              handle.resume();
                          });
    }
    auto coAwait(async_simple::Executor* executor) noexcept {
        return std::move(*this);
    }

private:
    asio::ip::tcp::socket& socket_;
    AsioBuffer buffer_;

    std::error_code ec_{};
    size_t size_{0};
};

export template <typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>> async_write(
    asio::ip::tcp::socket& socket, AsioBuffer&& buffer) noexcept {
    co_return co_await WriteAwaiter{socket, std::move(buffer)};
}

export class ConnectAwaiter {
public:
    ConnectAwaiter(asio::io_context& io_context, const std::string& host,
                   const std::string& port)
        : io_context_(io_context),
          socket_(io_context),
          host_(host),
          port_(port) {}
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::experimental::coroutine_handle<> handle) {
        asio::ip::tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, port_);
        asio::async_connect(socket_, endpoints,
                            [this, handle](std::error_code ec,
                                           asio::ip::tcp::endpoint) mutable {
                                ec_ = ec;
                                handle.resume();
                            });
    }
    auto await_resume() noexcept {
        return std::make_pair(ec_, std::move(socket_));
    }
    auto coAwait(async_simple::Executor* executor) noexcept {
        return std::move(*this);
    }

private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    std::string host_;
    std::string port_;
    std::error_code ec_{};
};

export inline async_simple::coro::Lazy<
    std::pair<std::error_code, asio::ip::tcp::socket>>
async_connect(asio::io_context& io_context, const std::string& host,
              const std::string& port) noexcept {
    co_return co_await ConnectAwaiter{io_context, host, port};
}