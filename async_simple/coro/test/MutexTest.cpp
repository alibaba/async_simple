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
#include "async_simple/coro/Mutex.h"
#include "async_simple/coro/Sleep.h"
#include "async_simple/executors/SimpleExecutor.h"

#include "async_simple/test/unittest.h"

using namespace std;

namespace async_simple {
namespace coro {

class MutexTest : public FUTURE_TESTBASE {
public:
    MutexTest() : _executor(4) {}
    void caseSetUp() override {}
    void caseTearDown() override {}

    executors::SimpleExecutor _executor;
};

TEST_F(MutexTest, testLock) {
    Mutex m;
    EXPECT_TRUE(m.tryLock());
    EXPECT_FALSE(m.tryLock());
    m.unlock();
    EXPECT_TRUE(m.tryLock());
}

TEST_F(MutexTest, testAsyncLock) {
    Mutex m;
    int value = 0;
    std::atomic<int> count = 2;

    auto writer = [&]() -> Lazy<void> {
        co_await m.coLock();
        value++;
        co_await async_simple::coro::sleep(1s);
        EXPECT_EQ(1, value);
        value--;
        m.unlock();
        count--;
    };

    writer().via(&_executor).detach();
    writer().via(&_executor).detach();
    while (count) {
    }

    EXPECT_EQ(0, value);
    count = 2;

    auto writer2 = [&]() -> Lazy<void> {
        {
            auto scopedLock = co_await m.coScopedLock();
            value++;
            co_await async_simple::coro::sleep(1s);
            EXPECT_EQ(1, value);
            value--;
        }
        // decrease count after the lock unlocked actually.
        count--;
    };

    writer2().via(&_executor).detach();
    writer2().via(&_executor).detach();
    while (count) {
    }

    EXPECT_EQ(0, value);
}

}  // namespace coro
}  // namespace async_simple
