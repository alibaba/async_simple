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
export module async_simple:coro.Util;

import std;

namespace async_simple {

namespace coro {

namespace detail {

// A detached coroutine. It would start to execute
// immediately and throws the exception it met.
// This could be used as the root of a coruotine
// execution chain.
//
// But the user shouldn't use this directly. It may be
// better to use `Lazy::start()`.
struct DetachedCoroutine {
    struct promise_type {
        std::suspend_never initial_suspend() noexcept {
            return {};
        }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() {
            try {
                std::rethrow_exception(std::current_exception());
            } catch (const std::exception& e) {
                std::fprintf(stderr, "find exception %s\n", e.what());
                std::fflush(stderr);
                std::rethrow_exception(std::current_exception());
            }
        }
        DetachedCoroutine get_return_object() noexcept {
            return DetachedCoroutine();
        }
    };
};

}  // namespace detail

// This allows we to co_await a non-awaitable. It would make
// the co_await expression to return directly.
template <typename T>
struct ReadyAwaiter {
    ReadyAwaiter(T value) : _value(std::move(value)) {}

    bool await_ready() const noexcept { return true; }
    void await_suspend(std::coroutine_handle<>) const noexcept {}
    T await_resume() noexcept { return std::move(_value); }

    T _value;
};

}  // namespace coro
}  // namespace async_simple
