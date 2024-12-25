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

#include <coroutine>
#include "async_simple/Try.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/executors/LocalExecutor.h"

#include "async_simple/test/unittest.h"

namespace async_simple {
namespace coro {

template <typename T>
struct CallbackAwaiter {
public:
    using CallbackFunction =
        std::function<void(std::coroutine_handle<>, std::function<void(T)>)>;

    CallbackAwaiter(CallbackFunction callback_function)
        : callback_function_(std::move(callback_function)) {}

    bool await_ready() noexcept { return false; }

    void await_suspend(std::coroutine_handle<> handle) {
        callback_function_(handle, [this](T t) { result_ = std::move(t); });
    }

    T await_resume() noexcept { return std::move(result_); }

private:
    CallbackFunction callback_function_;
    T result_;
};

Lazy<int> async_compute(int i) noexcept {
    int ret = co_await CallbackAwaiter<int>{
        [&, i](std::coroutine_handle<> handle, auto set_resume_value) {
            set_resume_value(i * 100);
            handle.resume();
        }};
    co_return ret;
}

Lazy<int> coro_compute() {
    int sum = 0;
    for (auto i = 0; i < 1000; ++i) {
        sum += co_await async_compute(i);
    }
    co_return sum;
}

TEST(LocalExecutorTest, testStackOverflow) {
    executors::LocalExecutor ex;
    int result{0};
    coro_compute().via(&ex).start([&](Try<int> i) { result = i.value(); });
    ex.Loop();
    EXPECT_EQ(result, 49950000);
}

}  // namespace coro
}  // namespace async_simple
