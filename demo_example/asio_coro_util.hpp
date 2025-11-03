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
#include <sys/types.h>
#include "async_simple/Executor.h"
#include "async_simple/Signal.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"

#include <chrono>
#include <concepts>
#include <cstdint>
#include "asio.hpp"

template <typename Arg, typename Derived>
class callback_awaitor_base {
private:
    template <typename Op>
    class callback_awaitor_impl {
    public:
        callback_awaitor_impl(Derived &awaitor, Op &op) noexcept
            : awaitor(awaitor), op(op) {}
        constexpr bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> handle) noexcept {
            awaitor.coro_ = handle;
            op(awaitor_handler{&awaitor});
        }
        auto coAwait(async_simple::Executor *executor) const noexcept {
            return *this;
        }
        decltype(auto) await_resume() noexcept {
            if constexpr (std::is_void_v<Arg>) {
                return;
            } else {
                return std::move(awaitor.arg_);
            }
        }

    private:
        Derived &awaitor;
        Op &op;
    };

public:
    class awaitor_handler {
    public:
        awaitor_handler(Derived *obj) : obj(obj) {}
        awaitor_handler(awaitor_handler &&) = default;
        awaitor_handler(const awaitor_handler &) = default;
        awaitor_handler &operator=(const awaitor_handler &) = default;
        awaitor_handler &operator=(awaitor_handler &&) = default;
        template <typename... Args>
        void set_value_then_resume(Args &&...args) const {
            set_value(std::forward<Args>(args)...);
            resume();
        }
        template <typename... Args>
        void set_value(Args &&...args) const {
            if constexpr (!std::is_void_v<Arg>) {
                obj->arg_ = {std::forward<Args>(args)...};
            }
        }
        void resume() const { obj->coro_.resume(); }

    private:
        Derived *obj;
    };
    template <typename Op>
    callback_awaitor_impl<Op> await_resume(Op &&op) noexcept {
        return callback_awaitor_impl<Op>{static_cast<Derived &>(*this), op};
    }

private:
    std::coroutine_handle<> coro_;
};

template <typename Arg>
class callback_awaitor
    : public callback_awaitor_base<Arg, callback_awaitor<Arg>> {
    friend class callback_awaitor_base<Arg, callback_awaitor<Arg>>;

private:
    Arg arg_;
};

template <>
class callback_awaitor<void>
    : public callback_awaitor_base<void, callback_awaitor<void>> {};

class period_timer : public asio::steady_timer {
public:
    using asio::steady_timer::steady_timer;
    template <typename T>
    period_timer(asio::io_context &ioc) : asio::steady_timer(ioc) {}

    async_simple::coro::Lazy<bool> async_await() noexcept {
        callback_awaitor<bool> awaitor;

        co_return co_await awaitor.await_resume([&](auto handler) {
            this->async_wait([&, handler](const auto &ec) {
                handler.set_value_then_resume(!ec);
            });
        });
    }
};

class AsioExecutor : public async_simple::Executor {
private:
    asio::io_context &executor_;

public:
    AsioExecutor(asio::io_context &executor) : executor_(executor) {}

    static asio::io_context **get_current() {
        static thread_local asio::io_context *current = nullptr;
        return &current;
    }

    virtual bool schedule(Func func) override {
        asio::dispatch(executor_, std::move(func));
        return true;
    }

    virtual bool schedule(Func func, uint64_t schedule_info) override {
        if ((schedule_info & 0xF) >=
            static_cast<uint64_t>(async_simple::Executor::Priority::YIELD)) {
            asio::post(executor_, std::move(func));
        } else {
            asio::dispatch(executor_, std::move(func));
        }
        return true;
    }

    virtual bool checkin(Func func, void *ctx) override {
        asio::dispatch(executor_, std::move(func));
        return true;
    }
    virtual void *checkout() override { return (void *)&executor_; }

    bool currentThreadInExecutor() const override {
        auto ctx = get_current();
        return *ctx == &executor_;
    }

    size_t currentContextId() const override {
        auto ctx = get_current();
        auto ptr = *ctx;
        return ptr ? (size_t)ptr : 0;
    }

private:
    void schedule(Func func, Duration dur) override {
        auto timer = std::make_unique<asio::steady_timer>(executor_, dur);
        auto tm = timer.get();
        tm->async_wait([fn = std::move(func),
                        timer = std::move(timer)](auto ec) { fn(); });
    }
};

template <typename T>
requires(!std::is_reference<T>::value) struct AsioCallbackAwaiter {
public:
    using CallbackFunction =
        std::function<void(std::coroutine_handle<>, std::function<void(T)>)>;

    AsioCallbackAwaiter(CallbackFunction callback_function)
        : callback_function_(std::move(callback_function)) {}

    bool await_ready() noexcept { return false; }

    void await_suspend(std::coroutine_handle<> handle) {
        callback_function_(handle, [this](T t) { result_ = std::move(t); });
    }

    auto coAwait(async_simple::Executor *executor) noexcept {
        return std::move(*this);
    }

    T await_resume() noexcept { return std::move(result_); }

private:
    CallbackFunction callback_function_;
    T result_;
};

inline async_simple::coro::Lazy<std::error_code> async_accept(
    asio::ip::tcp::acceptor &acceptor, asio::ip::tcp::socket &socket) noexcept {
    co_return co_await AsioCallbackAwaiter<std::error_code>{
        [&](std::coroutine_handle<> handle, auto set_resume_value) {
            acceptor.async_accept(
                socket, [handle, set_resume_value = std::move(
                                     set_resume_value)](auto ec) mutable {
                    set_resume_value(std::move(ec));
                    handle.resume();
                });
        }};
}

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_read_some(Socket &socket, AsioBuffer &&buffer) noexcept {
    co_return co_await AsioCallbackAwaiter<std::pair<std::error_code, size_t>>{
        [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
            socket.async_read_some(
                std::move(buffer),
                [handle, set_resume_value = std::move(set_resume_value)](
                    auto ec, auto size) mutable {
                    set_resume_value(std::make_pair(std::move(ec), size));
                    handle.resume();
                });
        }};
}

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>> async_read(
    Socket &socket, AsioBuffer &buffer) noexcept {
    co_return co_await AsioCallbackAwaiter<std::pair<std::error_code, size_t>>{
        [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
            asio::async_read(
                socket, buffer,
                [handle, set_resume_value = std::move(set_resume_value)](
                    auto ec, auto size) mutable {
                    set_resume_value(std::make_pair(std::move(ec), size));
                    handle.resume();
                });
        }};
}

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>>
async_read_until(Socket &socket, AsioBuffer &buffer,
                 asio::string_view delim) noexcept {
    co_return co_await AsioCallbackAwaiter<std::pair<std::error_code, size_t>>{
        [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
            asio::async_read_until(
                socket, buffer, delim,
                [handle, set_resume_value = std::move(set_resume_value)](
                    auto ec, auto size) mutable {
                    set_resume_value(std::make_pair(std::move(ec), size));
                    handle.resume();
                });
        }};
}

template <typename Socket, typename AsioBuffer>
inline async_simple::coro::Lazy<std::pair<std::error_code, size_t>> async_write(
    Socket &socket, AsioBuffer &&buffer) noexcept {
    co_return co_await AsioCallbackAwaiter<std::pair<std::error_code, size_t>>{
        [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
            asio::async_write(
                socket, std::move(buffer),
                [handle, set_resume_value = std::move(set_resume_value)](
                    auto ec, auto size) mutable {
                    set_resume_value(std::make_pair(std::move(ec), size));
                    handle.resume();
                });
        }};
}

inline async_simple::coro::Lazy<std::error_code> async_connect(
    asio::io_context &io_context, asio::ip::tcp::socket &socket,
    const std::string &host, const std::string &port) noexcept {
    co_return co_await AsioCallbackAwaiter<std::error_code>{
        [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
            asio::ip::tcp::resolver resolver(io_context);
            auto endpoints = resolver.resolve(host, port);
            asio::async_connect(
                socket, endpoints,
                [handle, set_resume_value = std::move(set_resume_value)](
                    auto ec, auto size) mutable {
                    set_resume_value(std::move(ec));
                    handle.resume();
                });
        }};
}

#endif
