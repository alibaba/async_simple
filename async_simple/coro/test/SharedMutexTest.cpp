/*
 * Copyright (c) 2023, Alibaba Group Holding Limited;
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
#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <random>
#include <thread>
#include <unordered_set>
#include "async_simple/Future.h"
#include "async_simple/coro/Collect.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SharedMutex.h"
#include "async_simple/coro/Sleep.h"
#include "async_simple/coro/SpinLock.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

using namespace std;

namespace async_simple {
namespace coro {

class SharedMutexTest : public FUTURE_TESTBASE {
public:
    SharedMutexTest() : _executor(4) {}
    void caseSetUp() override {}
    void caseTearDown() override {}

    executors::SimpleExecutor _executor;
};

TEST_F(SharedMutexTest, testTryLock) {
    SharedMutex m;
    EXPECT_TRUE(m.tryLock());
    EXPECT_FALSE(m.tryLock());
    syncAwait(m.unlock());
    EXPECT_TRUE(m.tryLock());
    syncAwait(m.unlock());
}

TEST_F(SharedMutexTest, testTryLockShared) {
    SharedMutex m;
    EXPECT_TRUE(m.tryLock());
    EXPECT_FALSE(m.tryLockShared());
    syncAwait(m.unlock());
    EXPECT_TRUE(m.tryLockShared());
    EXPECT_TRUE(m.tryLockShared());
    syncAwait(m.unlockShared());
    syncAwait(m.unlockShared());
}

TEST_F(SharedMutexTest, testLockShared) {
    SharedMutex m;

    std::default_random_engine e;
    std::uniform_int_distribution<int> rnd(0, 100000);
    std::atomic<int64_t> sum = 0;
    std::set<int> table;
    for (int i = 0; i < 100000; ++i) {
        table.insert(rnd(e));
    }

    std::atomic<std::set<int>::size_type> value_for_check = table.size();
    constexpr int writer_count = 1000;
    constexpr int reader_count = 1000;

    auto writer = [&]() -> Lazy<void> {
        co_await m.coLock();
        value_for_check -= table.erase(rnd(e));
        co_await m.unlock();
    };
    auto reader = [&]() -> Lazy<void> {
        co_await m.coLockShared();
        constexpr int read_count_max = 100;
        for (int i = 0; i < read_count_max; ++i) {
            auto iter = table.find(rnd(e));
            if (iter != table.end()) {
                sum += *iter;
            }
            EXPECT_TRUE(table.size() == value_for_check);
        }
        co_await m.unlockShared();
    };
    std::vector<Lazy<void>> works;

    for (int i = 0; i < reader_count; ++i)
        works.emplace_back(reader());
    for (int i = 0; i < writer_count; ++i)
        works.emplace_back(writer());
    auto f = [works_ = std::move(works)]() mutable -> Lazy<void> {
        [[maybe_unused]] auto ret = co_await collectAllPara(std::move(works_));
        co_return;
    };
    syncAwait(f().via(&_executor));

    std::cout << "sum: " << sum << std::endl;
}

}  // namespace coro
}  // namespace async_simple
