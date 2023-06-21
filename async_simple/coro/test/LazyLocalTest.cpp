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
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

#include <condition_variable>
#include <mutex>

namespace async_simple::coro {

TEST(LazyLocalTest, testLazyLocal) {
    int* i = new int(10);
    async_simple::executors::SimpleExecutor ex(2);

    auto sub_task = [&]() -> Lazy<> {
        int* v = co_await LazyLocals<int>{};
        EXPECT_EQ(v, i);
        EXPECT_EQ(*v, 20);
        *v = 30;
    };

    auto task = [&]() -> Lazy<> {
        void* v = co_await LazyLocals{};
        EXPECT_EQ(v, i);
        (*i) = 20;
        co_await sub_task();
        co_return;
    };
    syncAwait(task().via(&ex).setLazyLocal(i));
    EXPECT_EQ(*i, 30);

    std::condition_variable cv;
    std::mutex mut;
    bool done = false;
    task().via(&ex).setLazyLocal(i).start([&](Try<void>) {
        std::unique_lock<std::mutex> lock(mut);
        EXPECT_EQ(*i, 30);
        delete i;
        i = nullptr;
        done = true;
        cv.notify_all();
    });
    std::unique_lock<std::mutex> lock(mut);
    cv.wait(lock, [&]() -> bool { return done == true; });
    EXPECT_EQ(i, nullptr);
}

}  // namespace async_simple::coro
