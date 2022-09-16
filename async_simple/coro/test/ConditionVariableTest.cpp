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
#include <async_simple/coro/ConditionVariable.h>
#include <async_simple/coro/Lazy.h>
#include <async_simple/coro/SpinLock.h>
#include <async_simple/executors/SimpleExecutor.h>
#include <async_simple/test/unittest.h>
#include <chrono>

using namespace std::chrono_literals;

namespace async_simple {
namespace coro {

#define CHECK_EXECUTOR(ex)                                            \
    do {                                                              \
        EXPECT_TRUE((ex)->currentThreadInExecutor()) << (ex)->name(); \
        auto current = co_await CurrentExecutor();                    \
        EXPECT_EQ((ex), current) << (ex)->name();                     \
    } while (0)

class ConditionVariableTest : public FUTURE_TESTBASE {
public:
    ConditionVariableTest() : _executor(2) {}
    void caseSetUp() override {}
    void caseTearDown() override {}

    executors::SimpleExecutor _executor;
};

TEST_F(ConditionVariableTest, testSingleWait) {
    auto data = 0;
    Notifier notifier;
    std::atomic<size_t> latch(2);

    executors::SimpleExecutor e1(1);
    executors::SimpleExecutor e2(1);

    auto producer = [&]() -> Lazy<void> {
        data = 1;
        CHECK_EXECUTOR(&e2);
        notifier.notify();
        CHECK_EXECUTOR(&e2);
        latch.fetch_sub(1u, std::memory_order_relaxed);
        co_return;
    };
    auto awaiter = [&]() -> Lazy<void> {
        CHECK_EXECUTOR(&e1);
        co_await notifier.wait();
        CHECK_EXECUTOR(&e1);
        EXPECT_EQ(1, data);
        data = 2;
        latch.fetch_sub(1u, std::memory_order_relaxed);
        co_return;
    };
    awaiter().via(&e1).start([](Try<void> var) {});
    producer().via(&e2).start([](Try<void> var) {});
    while (latch.load(std::memory_order_relaxed))
        ;
    EXPECT_EQ(2, data);
}

TEST_F(ConditionVariableTest, testMultiWait) {
    auto data = 0;
    Notifier notifier;

    std::atomic<int> barrier(0);
    auto producer = [&]() -> Lazy<void> {
        std::this_thread::sleep_for(100us);
        data = 1;
        notifier.notify();
        co_return;
    };
    auto awaiter1 = [&]() -> Lazy<void> {
        co_await notifier.wait();
        EXPECT_EQ(1, data);
        barrier.fetch_add(1, std::memory_order_relaxed);
        co_return;
    };
    auto awaiter2 = [&]() -> Lazy<void> {
        co_await notifier.wait();
        EXPECT_EQ(1, data);
        barrier.fetch_add(1, std::memory_order_relaxed);
        co_return;
    };
    awaiter2().via(&_executor).start([](Try<void> var) {});
    awaiter1().via(&_executor).start([](Try<void> var) {});
    producer().via(&_executor).start([](Try<void> var) {});
    // spin wait
    while (barrier.load(std::memory_order_relaxed) < 2)
        ;

    notifier.reset();
    std::atomic<size_t> latch(2);
    auto producer2 = [&]() -> Lazy<void> {
        data = 0;
        notifier.notify();
        latch.fetch_sub(1u, std::memory_order_relaxed);
        co_return;
    };
    auto awaiter3 = [&]() -> Lazy<void> {
        co_await notifier.wait();
        EXPECT_EQ(0, data);
        latch.fetch_sub(1u, std::memory_order_relaxed);
        co_return;
    };
    producer2().via(&_executor).start([](Try<void> var) {});
    awaiter3().via(&_executor).start([](Try<void> var) {});
    while (latch.load(std::memory_order_relaxed))
        ;
    EXPECT_EQ(0, data);
}

TEST_F(ConditionVariableTest, testSingleWaitPredicate) {
    SpinLock mutex;
    ConditionVariable<SpinLock> cv;
    auto var = 0;

    executors::SimpleExecutor e1(1);
    executors::SimpleExecutor e2(1);
    std::atomic<size_t> latch(2);

    auto producer = [&]() -> Lazy<void> {
        CHECK_EXECUTOR(&e2);
        co_await mutex.coLock();
        CHECK_EXECUTOR(&e2);
        var = 1;
        cv.notify();
        CHECK_EXECUTOR(&e2);
        std::this_thread::sleep_for(500us);
        mutex.unlock();
        CHECK_EXECUTOR(&e2);
        latch.fetch_sub(1u, std::memory_order_relaxed);
        co_return;
    };
    auto awaiter = [&]() -> Lazy<void> {
        CHECK_EXECUTOR(&e1);
        co_await mutex.coLock();
        CHECK_EXECUTOR(&e1);
        co_await cv.wait(mutex, [&] { return var > 0; });
        CHECK_EXECUTOR(&e1);
        mutex.unlock();
        CHECK_EXECUTOR(&e1);
        EXPECT_EQ(1, var);
        latch.fetch_sub(1u, std::memory_order_relaxed);
        co_return;
    };
    awaiter().via(&e1).start([](Try<void> var) {});
    producer().via(&e2).start([](Try<void> var) {});
    while (latch.load(std::memory_order_relaxed))
        ;
}

TEST_F(ConditionVariableTest, testSingleWaitPredicateWithScopeLock) {
    SpinLock mutex;
    ConditionVariable<SpinLock> cv;
    auto var = 0;

    std::atomic<size_t> latch(2);
    auto producer = [&]() -> Lazy<void> {
        auto scoper = co_await mutex.coScopedLock();
        var = 1;
        cv.notify();
        co_return;
    };
    auto awaiter = [&]() -> Lazy<void> {
        auto scoper = co_await mutex.coScopedLock();
        co_await cv.wait(mutex, [&] { return var > 0; });
        EXPECT_EQ(1, var);
        co_return;
    };
    awaiter().via(&_executor).start([&](Try<void> var) {
        latch.fetch_sub(1u, std::memory_order_relaxed);
    });
    producer().via(&_executor).start([&](Try<void> var) {
        latch.fetch_sub(1u, std::memory_order_relaxed);
    });
    while (latch.load(std::memory_order_relaxed))
        ;
}

}  // namespace coro
}  // namespace async_simple
