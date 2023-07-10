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

#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/coro/UpdatePriority.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

namespace async_simple::coro {

TEST(UpdatePriorityTest, testUpdatePriority) {
    async_simple::executors::SimpleExecutor executor(2);
    int8_t old_priority = 0;
    int8_t new_priority = 0;
    auto task = [&]() -> async_simple::coro::Lazy<void> {
        old_priority = co_await Priority{};
        co_await updatePriority(100, UpdatePriorityOption::DELAYED);
        new_priority = co_await Priority{};
        co_return;
    };
    async_simple::coro::syncAwait(task().via(&executor).setPriority(20));
    EXPECT_EQ(old_priority, 20);
    EXPECT_EQ(new_priority, 100);
}

}  // namespace async_simple::coro
