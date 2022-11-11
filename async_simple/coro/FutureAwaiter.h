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
#ifndef ASYNC_SIMPLE_CORO_FUTURE_AWAITER_H
#define ASYNC_SIMPLE_CORO_FUTURE_AWAITER_H

#include "async_simple/Future.h"
#include "async_simple/experimental/coroutine.h"

namespace async_simple {

template <typename T>
auto operator co_await(Future<T>&& future) {
    struct FutureAwaiter {
        Future<T> future_;

        bool await_ready() { return future_.hasResult(); }
        void await_suspend(coro::CoroHandle<> continuation) {
            future_.setContinuation(
                [continuation](Try<T>&& t) mutable { continuation.resume(); });
        }
        auto await_resume() { return std::move(future_.value()); }
    };

    return FutureAwaiter{std::move(future)};
}

template <typename T>
[[deprecated("Require an rvalue future.")]]
auto operator co_await(T&& future) requires IsFuture<std::decay_t<T>>::value {
    return std::move(operator co_await(std::move(future)));
}

}  // namespace async_simple

#endif
