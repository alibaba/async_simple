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
#include <async_simple/coro/FutureAwaiter.h>
#include <async_simple/coro/Lazy.h>
#include <async_simple/coro/SyncAwait.h>
#include <async_simple/test/unittest.h>
#include <chrono>
#include <thread>

namespace async_simple {
namespace coro {

class FutureAwaiterTest : public FUTURE_TESTBASE {
public:
    FutureAwaiterTest() = default;
    void caseSetUp() override {}
    void caseTearDown() override {}

    template <typename Callback>
    void sum(int a, int b, Callback&& callback) const {
        std::thread([callback = std::move(callback), a, b]() mutable {
            callback(a + b);
        }).detach();
    }
};

TEST_F(FutureAwaiterTest, testWithFuture) {
    auto lazy1 = [&]() -> Lazy<> {
        Promise<int> pr;
        auto fut = pr.getFuture();
        sum(1, 1, [pr = std::move(pr)](int val) mutable { pr.setValue(val); });
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto val = co_await fut;
        EXPECT_EQ(2, val);
    };
    syncAwait(lazy1());
    auto lazy2 = [&]() -> Lazy<> {
        Promise<int> pr;
        auto fut = pr.getFuture();
        sum(1, 1, [pr = std::move(pr)](int val) mutable {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            pr.setValue(val);
        });
        auto val = co_await std::move(fut);
        EXPECT_EQ(2, val);
    };
    syncAwait(lazy2());
}

}  // namespace coro
}  // namespace async_simple
