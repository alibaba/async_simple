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
#include <cstdint>
#include <functional>

#include "async_simple/Cancellation.h"
#include "async_simple/Executor.h"
#include "async_simple/coro/Collect.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/Sleep.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using namespace std::chrono_literals;

namespace async_simple {

namespace coro {

class SleepTest : public FUTURE_TESTBASE {
public:
    void caseSetUp() override {}
    void caseTearDown() override {}
};

TEST_F(SleepTest, testSleep) {
    executors::SimpleExecutor e1(5);

    auto sleepTask = [&]() -> Lazy<> {
        auto current = co_await CurrentExecutor();
        EXPECT_EQ(&e1, current);

        auto startTime = std::chrono::system_clock::now();
        co_await coro::sleep(1s);
        auto endTime = std::chrono::system_clock::now();

        current = co_await CurrentExecutor();
        EXPECT_EQ(&e1, current);

        auto duration = endTime - startTime;
        cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                    .count()
             << endl;
    };

    syncAwait(sleepTask().via(&e1));

    auto sleepTask2 = [&]() -> Lazy<> {
        auto current = co_await CurrentExecutor();
        EXPECT_EQ(&e1, current);

        auto startTime = std::chrono::system_clock::now();
        co_await coro::sleep(900ms);
        auto endTime = std::chrono::system_clock::now();

        current = co_await CurrentExecutor();
        EXPECT_EQ(&e1, current);

        auto duration = endTime - startTime;
        cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                    .count()
             << endl;
    };

    syncAwait(sleepTask2().via(&e1));

    auto sleepTask3 = [&]() -> Lazy<> {
        class Executor : public async_simple::Executor {
        public:
            Executor() = default;
            virtual bool schedule(Func) override { return true; }
            virtual void schedule(Func func, Duration dur) override {
                std::thread([this, func = std::move(func), dur]() {
                    id = std::this_thread::get_id();
                    std::this_thread::sleep_for(dur);
                    func();
                }).detach();
            }
            virtual void schedule(Func func, Duration dur, uint64_t,
                                  CancellationSlot*) override {
                schedule(std::move(func), dur);
            }

            std::thread::id id;
        };
        Executor ex;
        auto startTime = std::chrono::system_clock::now();
        co_await coro::sleep(&ex, 900ms);
        auto endTime = std::chrono::system_clock::now();

        std::cout << std::this_thread::get_id() << std::endl;
        std::cout << ex.id << std::endl;

        EXPECT_EQ(std::this_thread::get_id(), ex.id);

        auto duration = endTime - startTime;
        cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                    .count()
             << endl;
    };

    syncAwait(sleepTask3().via(&e1));
}

Lazy<void> cancelSleep() {
    auto executor = co_await CurrentExecutor{};
    auto sleep = [](int i, std::atomic<int>& cnt) -> Lazy<void> {
        bool cancel_flag = false;
        try {
            co_await async_simple::coro::sleep(i * 1s);
        } catch (const std::system_error& err) {
            ++cnt;
            cancel_flag = true;
        }
        auto slot = co_await async_simple::coro::CurrentCancellationSlot{};
        auto ok = slot->signal()->emit(CancellationType::terminal);
        if (ok) {
            std::cout << "Coro " << i << " emit cancel work" << std::endl;
        } else {
            EXPECT_TRUE(cancel_flag);
        }
        co_return;
    };
    {
        std::vector<RescheduleLazy<void>> works;
        std::atomic<int> cnt;
        auto tp1 = std::chrono::steady_clock::now();
        for (int i = 0; i < 100; ++i) {
            works.emplace_back(sleep(i, cnt).via(executor));
        }
        co_await async_simple::coro::collectAll(std::move(works));
        auto tp2 = std::chrono::steady_clock::now();
        EXPECT_EQ(cnt, 99);
        std::cout << "cost time: " << (tp2 - tp1) / 1ms << "ms" << std::endl;
        EXPECT_LE((tp2 - tp1) / 1ms, 800);
    }
    {
        std::vector<RescheduleLazy<void>> works;
        std::atomic<int> cnt;
        auto tp1 = std::chrono::steady_clock::now();
        for (int i = 99; i >= 0; --i) {
            works.emplace_back(sleep(i, cnt).via(executor));
        }
        co_await async_simple::coro::collectAll(std::move(works));
        auto tp2 = std::chrono::steady_clock::now();
        EXPECT_EQ(cnt, 99);
        std::cout << "cost time: " << (tp2 - tp1) / 1ms << "ms" << std::endl;
        EXPECT_LE((tp2 - tp1) / 1ms, 800);
    }
}

TEST_F(SleepTest, testCancelSleep) {
    executors::SimpleExecutor e1(5);
    syncAwait(cancelSleep().via(&e1));
}

}  // namespace coro
}  // namespace async_simple
