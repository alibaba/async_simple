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

#include "async_simple/coro/Dispatch.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

namespace async_simple::coro {

struct alignas(4) AlignTest {
    int i;
};

TEST(DispatchTest, testDispatch) {
    async_simple::executors::SimpleExecutor executor1(2);
    async_simple::executors::SimpleExecutor executor2(2);
    auto task = [&]() -> Lazy<> {
        Executor* ex = co_await CurrentExecutor{};
        EXPECT_EQ(ex, &executor1);
        co_await dispatch(&executor1);
        ex = co_await CurrentExecutor{};
        EXPECT_EQ(ex, &executor1);
        co_await dispatch(&executor2);
        ex = co_await CurrentExecutor{};
        EXPECT_EQ(ex, &executor2);
    };
    async_simple::coro::syncAwait(task().via(&executor1));

    auto task1 = [&]() -> Lazy<AlignTest> {
        Executor* ex = co_await CurrentExecutor{};
        EXPECT_EQ(ex, &executor1);
        co_await dispatch(&executor2);
        ex = co_await CurrentExecutor{};
        EXPECT_EQ(ex, &executor2);
        co_return AlignTest{2};
    };

    auto task2 = [&]() -> Lazy<> {
        Executor* ex = co_await CurrentExecutor{};
        EXPECT_EQ(ex, &executor1);
        AlignTest t = co_await task1();
        EXPECT_EQ(t.i, 2);
        ex = co_await CurrentExecutor{};
        EXPECT_EQ(ex, &executor2);
    };
    async_simple::coro::syncAwait(task2().via(&executor1));
}

}  // namespace async_simple::coro
