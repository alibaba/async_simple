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
#include <algorithm>
import std;
#include "../unittest.h"

import async_simple;


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
    // We can't use std::make_shared in libstdc++12 due to some reasons.
    _tp = shared_ptr<async_simple::util::ThreadPool>(new async_simple::util::ThreadPool(2));
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
    EXPECT_GE(pool.getItemCount(), 0);

    EXPECT_EQ(ThreadPool::ERROR_TYPE::ERROR_POOL_ITEM_IS_NULL,
              pool.scheduleById(nullptr));
    EXPECT_EQ(pool.getCurrentId(), -1);

    pool.scheduleById([&pool] { EXPECT_EQ(pool.getCurrentId(), 1); }, 1);
}

TEST(ThreadTest, BasicTest) {
    ThreadPool pool;
    EXPECT_EQ(ThreadPool::DEFAULT_THREAD_NUM, pool.getThreadNum());
    ThreadPool pool1(2);
    EXPECT_EQ(2, pool1.getThreadNum());

    TestBasic(pool);

    ThreadPool tp(ThreadPool::DEFAULT_THREAD_NUM, /*enableWorkSteal = */ true);
    TestBasic(tp);
}

class ScopedTimer {
public:
    ScopedTimer(const char* name)
        : m_name(name), m_beg(std::chrono::high_resolution_clock::now()) {}
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto dur =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_beg);
        std::cout << m_name << " : " << dur.count() << " ns\n";
    }

private:
    const char* m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_beg;
};

const unsigned int COUNT = 1'000'000;
const unsigned int REPS = 10;

void Benchmark(bool enableWorkSteal) {
    std::atomic<int> count = 0;
    {
        ThreadPool tp(ThreadPool::DEFAULT_THREAD_NUM, enableWorkSteal);
        ScopedTimer timer("ThreadPool");

        for (int i = 0; i < COUNT; ++i) {
            auto ret = tp.scheduleById([i, &count] {
                count++;
                int x;
                auto reps = REPS + (REPS * (rand() % 5));
                for (int n = 0; n < reps; ++n)
                    x = i + rand();
                (void)x;
            });
            EXPECT_EQ(ret, ThreadPool::ERROR_TYPE::ERROR_NONE);
        }
    }
    EXPECT_EQ(count, COUNT);
}

TEST(ThreadTest, BenchmarkTest) {
    Benchmark(/*enableWorkSteal = */ false);
    Benchmark(/*enableWorkSteal = */ true);
}

}  // namespace async_simple
