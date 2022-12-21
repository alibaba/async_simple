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

#include "async_simple/coro/Semaphore.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

using namespace std::chrono_literals;

namespace async_simple::coro {

#define CHECK_EXECUTOR(ex)                                            \
    do {                                                              \
        EXPECT_TRUE((ex)->currentThreadInExecutor()) << (ex)->name(); \
        auto current = co_await CurrentExecutor();                    \
        EXPECT_EQ((ex), current) << (ex)->name();                     \
    } while (0)

class SemahoreTest : public FUTURE_TESTBASE {
public:
    SemahoreTest() : _executor(4) {}
    void caseSetUp() override {}
    void caseTearDown() override {}

    executors::SimpleExecutor _executor;
};

// Same as ConditionVariable testSingleWait
TEST_F(SemahoreTest, testSingleWait) {
    auto data = 0;
    BinarySemaphore notifier(0);
    std::atomic<size_t> latch(2);

    executors::SimpleExecutor e1(1);
    executors::SimpleExecutor e2(1);

    auto producer = [&]() -> Lazy<void> {
        data = 1;
        CHECK_EXECUTOR(&e2);
        co_await notifier.release();
        CHECK_EXECUTOR(&e2);
        co_return;
    };
    auto awaiter = [&]() -> Lazy<void> {
        CHECK_EXECUTOR(&e1);
        co_await notifier.acquire();
        CHECK_EXECUTOR(&e1);
        EXPECT_EQ(1, data);
        data = 2;
        co_return;
    };
    awaiter().via(&e1).start(
        [&](Try<void> var) { latch.fetch_sub(1u, std::memory_order_relaxed); });
    producer().via(&e2).start(
        [&](Try<void> var) { latch.fetch_sub(1u, std::memory_order_relaxed); });
    while (latch.load(std::memory_order_relaxed))
        ;
    EXPECT_EQ(2, data);
}

// Same as ConditionVariable testMultiWait
TEST_F(SemahoreTest, testMultiWait) {
    auto data = 0;
    CountingSemaphore<2> notifier(0);

    std::atomic<int> barrier(0);
    auto producer = [&]() -> Lazy<void> {
        std::this_thread::sleep_for(100us);
        data = 1;
        co_await notifier.release(notifier.max());
        co_return;
    };
    auto awaiter1 = [&]() -> Lazy<void> {
        co_await notifier.acquire();
        EXPECT_EQ(1, data);
        co_return;
    };
    auto awaiter2 = [&]() -> Lazy<void> {
        co_await notifier.acquire();
        EXPECT_EQ(1, data);
        co_return;
    };
    awaiter2().via(&_executor).start([&](Try<void> var) {
        barrier.fetch_add(1, std::memory_order_relaxed);
    });
    awaiter1().via(&_executor).start([&](Try<void> var) {
        barrier.fetch_add(1, std::memory_order_relaxed);
    });
    producer().via(&_executor).start([](Try<void> var) {});
    // spin wait
    while (barrier.load(std::memory_order_relaxed) < 2)
        ;

    std::atomic<size_t> latch(2);
    auto producer2 = [&]() -> Lazy<void> {
        data = 0;
        co_await notifier.release();
        co_return;
    };
    auto awaiter3 = [&]() -> Lazy<void> {
        co_await notifier.acquire();
        EXPECT_EQ(0, data);
        co_return;
    };
    producer2().via(&_executor).start([&](Try<void> var) {
        latch.fetch_sub(1u, std::memory_order_relaxed);
    });
    awaiter3().via(&_executor).start([&](Try<void> var) {
        latch.fetch_sub(1u, std::memory_order_relaxed);
    });
    while (latch.load(std::memory_order_relaxed))
        ;
    EXPECT_EQ(0, data);
}

// FIXME: macos release model Bus error
#ifndef __APPLE__
TEST_F(SemahoreTest, testAsMutex) {
    std::atomic<size_t> latch(2);
    BinarySemaphore sem(1);
    int count = 0;
    auto lazy1 = [&]() -> Lazy<> {
        for (int i = 0; i < 5000; ++i) {
            co_await sem.acquire();
            count++;
            co_await sem.release();
        }
        co_return;
    };
    auto lazy2 = [&]() -> Lazy<> {
        for (int i = 0; i < 5000; ++i) {
            co_await sem.acquire();
            count--;
            co_await sem.release();
        }
        co_return;
    };

    lazy1().via(&_executor).start([&](auto&&) {
        latch.fetch_sub(1u, std::memory_order_relaxed);
    });
    lazy2().via(&_executor).start([&](auto&&) {
        latch.fetch_sub(1u, std::memory_order_relaxed);
    });

    while (latch.load(std::memory_order_relaxed)) {
    }
    EXPECT_EQ(count, 0);
}
#endif

}  // namespace async_simple::coro
