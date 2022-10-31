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
#include <gtest/gtest.h>
#include <exception>

#include <chrono>
#include <functional>
#include <thread>
#include <vector>
#include "async_simple/test/unittest.h"
#include "async_simple/util/ThreadPool.h"

using namespace std;

namespace async_simple {

class ThreadPoolTest : public FUTURE_TESTBASE {
public:
    shared_ptr<async_simple::util::ThreadPool> _tp;

public:
    void caseSetUp() override {}
    void caseTearDown() override {}
};

TEST_F(ThreadPoolTest, testScheduleWithId) {
    _tp = make_shared<async_simple::util::ThreadPool>(2);
    std::thread::id id1, id2, id3;
    std::atomic<bool> done1(false), done2(false), done3(false), done4(false);
    std::function<void()> f1 = [this, &done1, &id1]() {
        id1 = std::this_thread::get_id();
        ASSERT_EQ(_tp->getCurrentId(), 0);
        done1 = true;
    };
    std::function<void()> f2 = [this, &done2, &id2]() {
        id2 = std::this_thread::get_id();
        ASSERT_EQ(_tp->getCurrentId(), 0);
        done2 = true;
    };
    std::function<void()> f3 = [this, &done3, &id3]() {
        id3 = std::this_thread::get_id();
        ASSERT_EQ(_tp->getCurrentId(), 1);
        done3 = true;
    };
    std::function<void()> f4 = [&done4]() { done4 = true; };
    _tp->scheduleById(std::move(f1), 0);
    _tp->scheduleById(std::move(f2), 0);
    _tp->scheduleById(std::move(f3), 1);
    _tp->scheduleById(std::move(f4));

    while (!done1.load() || !done2.load() || !done3.load() || !done4.load())
        ;
    ASSERT_TRUE(id1 == id2) << id1 << " " << id2;
    ASSERT_TRUE(id1 != id3) << id1 << " " << id3;
    ASSERT_TRUE(_tp->getCurrentId() == -1);
}

using namespace async_simple::util;

void TestBasic(ThreadPool& pool) {
    EXPECT_EQ(ThreadPool::ERROR_TYPE::ERROR_NONE, pool.scheduleById([] {}));
    EXPECT_GE(pool.getItemCount(), 0u);

    EXPECT_EQ(ThreadPool::ERROR_TYPE::ERROR_POOL_ITEM_IS_NULL,
              pool.scheduleById(nullptr));
    EXPECT_EQ(pool.getCurrentId(), -1);

    pool.scheduleById([&pool] { EXPECT_EQ(pool.getCurrentId(), 0); }, 0);
}

TEST(ThreadTest, BasicTest) {
    ThreadPool pool;
    EXPECT_EQ(std::thread::hardware_concurrency(),
              static_cast<decltype(std::thread::hardware_concurrency())>(
                  pool.getThreadNum()));
    ThreadPool pool1(2);
    EXPECT_EQ(2, pool1.getThreadNum());

    TestBasic(pool);

    ThreadPool tp(std::thread::hardware_concurrency(),
                  /*enableWorkSteal = */ true);
    TestBasic(tp);
}

}  // namespace async_simple
