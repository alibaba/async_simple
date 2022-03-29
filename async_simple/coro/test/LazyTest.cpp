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
#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <type_traits>

#include <iostream>

#include <async_simple/coro/Lazy.h>
#include <async_simple/coro/test/Time.h>
#include <async_simple/executors/SimpleExecutor.h>
#include <async_simple/experimental/coroutine.h>
#include <async_simple/test/unittest.h>
#include <async_simple/util/Condition.h>
#include <chrono>

using namespace std;
using namespace std::chrono_literals;

namespace async_simple {
namespace coro {

#define CHECK_EXECUTOR(ex)                                            \
    do {                                                              \
        EXPECT_TRUE((ex)->currentThreadInExecutor()) << (ex)->name(); \
        auto current = co_await CurrentExecutor();                    \
        EXPECT_EQ((ex), current) << (ex)->name();                     \
    } while (0)

class LazyTest : public FUTURE_TESTBASE {
public:
    LazyTest() : _executor(1) {}
    void caseSetUp() override {
        std::lock_guard<std::mutex> g(_mtx);
        _value = 0;
        _done = false;
    }
    void caseTearDown() override {}

    std::mutex _mtx;
    std::condition_variable _cv;
    int _value;
    bool _done;

    executors::SimpleExecutor _executor;

public:
    void applyValue(std::function<void(int x)> f) {
        std::thread([this, f = std::move(f)]() {
            std::unique_lock<std::mutex> l(_mtx);
            _cv.wait(l, [this]() { return _done; });
            f(_value);
        }).detach();
    }

    Lazy<int> getValue() {
        struct ValueAwaiter {
            LazyTest* test;
            int value;
            ValueAwaiter(LazyTest* t) : test(t), value(0) {}

            bool await_ready() { return false; }
            void await_suspend(
                STD_CORO::coroutine_handle<> continuation) noexcept {
                test->applyValue(
                    [this, c = std::move(continuation)](int v) mutable {
                        value = v;
                        c.resume();
                    });
            }
            int await_resume() noexcept { return value; }
        };
        co_return co_await ValueAwaiter(this);
    }

    Lazy<void> testExcept() {
        throw std::runtime_error("testExcept test");
        co_return;
    };

    Lazy<void> makeVoidTask() {
        struct ValueAwaiter {
            ValueAwaiter() {}

            bool await_ready() { return false; }
            void await_suspend(
                STD_CORO::coroutine_handle<> continuation) noexcept {
                std::thread([c = continuation]() mutable {
                    c.resume();
                }).detach();
            }
            void await_resume() noexcept {}
        };
        auto id1 = std::this_thread::get_id();
        co_await ValueAwaiter();
        auto id2 = std::this_thread::get_id();
        EXPECT_EQ(id1, id2);
    }

    template <typename T>
    Lazy<T> getValue(T x) {
        struct ValueAwaiter {
            T value;
            ValueAwaiter(T v) : value(v) {}

            bool await_ready() { return false; }
            void await_suspend(
                STD_CORO::coroutine_handle<> continuation) noexcept {
                std::thread([c = continuation]() mutable {
                    c.resume();
                }).detach();
            }
            T await_resume() noexcept { return value; }
        };
        auto id1 = std::this_thread::get_id();
        auto ret = co_await ValueAwaiter(x);
        auto id2 = std::this_thread::get_id();
        EXPECT_EQ(id1, id2);
        co_return ret;
    }

    template <bool thread_id = false>
    Lazy<int> getValueWithSleep(int x) {
        struct ValueAwaiter {
            int value;
            ValueAwaiter(int v) : value(v) {}

            bool await_ready() { return false; }
            void await_suspend(
                STD_CORO::coroutine_handle<> continuation) noexcept {
                std::thread([c = continuation]() mutable {
                    std::this_thread::sleep_for(
                        std::chrono::microseconds(rand() % 1000 + 1));
                    c.resume();
                }).detach();
            }
            int await_resume() noexcept { return value; }
        };
        auto id1 = std::this_thread::get_id();
        auto ret = co_await ValueAwaiter(x);
        auto id2 = std::this_thread::get_id();
        EXPECT_EQ(id1, id2);
        co_return ret;
    }

    Lazy<std::thread::id> getThreadId() {
        struct ValueAwaiter {
            ValueAwaiter() {}
            bool await_ready() { return false; }
            void await_suspend(
                STD_CORO::coroutine_handle<> continuation) noexcept {
                std::thread([c = continuation]() mutable {
                    std::this_thread::sleep_for(
                        std::chrono::microseconds(rand() % 1000000 + 1));
                    c.resume();
                }).detach();
            }
            void await_resume() noexcept {}
        };
        auto id1 = std::this_thread::get_id();
        co_await ValueAwaiter();
        auto id2 = std::this_thread::get_id();
        EXPECT_EQ(id1, id2);
        co_return id1;
    }

    Lazy<int> plusOne() {
        int v = co_await getValue();
        co_return v + 1;
    }

    Lazy<int> testFunc() { co_return 3; }

    void triggerValue(int val) {
        std::unique_lock<std::mutex> lock(_mtx);
        _value = val;
        _done = true;
        _cv.notify_one();
    }
};

TEST_F(LazyTest, testSimpleAsync) {
    auto test = [this]() -> Lazy<int> {
        CHECK_EXECUTOR(&_executor);
        auto ret = co_await plusOne();
        CHECK_EXECUTOR(&_executor);
        co_return ret;
    };
    triggerValue(100);
    ASSERT_EQ(101, syncAwait(test().via(&_executor)));
}

TEST_F(LazyTest, testVia) {
    auto test = [this]() -> Lazy<int> {
        auto tid = std::this_thread::get_id();
        auto ret = co_await plusOne();
        EXPECT_TRUE(_executor.currentThreadInExecutor());
        EXPECT_TRUE(tid == std::this_thread::get_id());
        co_return ret;
    };
    triggerValue(100);
    auto ret = syncAwait(test().via(&_executor));
    ASSERT_EQ(101, ret);
}

TEST_F(LazyTest, testNoVia) {
    auto test = [this]() -> Lazy<int> { co_return co_await testFunc(); };
    ASSERT_EQ(3, syncAwait(test()));
}

TEST_F(LazyTest, testYield) {
    executors::SimpleExecutor executor(1);
    std::mutex m1;
    std::mutex m2;
    int value1 = 0;
    int value2 = 0;
    m1.lock();
    m2.lock();

    auto test1 = [](std::mutex& m, int& value) -> Lazy<void> {
        m.lock();
        // push task to queue's tail
        co_await Yield();
        value++;
        co_return;
    };

    auto test2 = [](std::mutex& m, int& value) -> Lazy<void> {
        m.lock();
        value++;
        co_return;
    };

    test1(m1, value1).via(&executor).start([](Try<void> result) {});
    std::this_thread::sleep_for(100000us);
    ASSERT_EQ(0, value1);

    test2(m2, value2).via(&executor).start([](Try<void> result) {});
    std::this_thread::sleep_for(100000us);
    ASSERT_EQ(0, value2);

    m1.unlock();
    std::this_thread::sleep_for(100000us);
    ASSERT_EQ(0, value1);
    ASSERT_EQ(0, value2);

    m2.unlock();
    std::this_thread::sleep_for(100000us);
    ASSERT_EQ(1, value1);
    ASSERT_EQ(1, value2);

    m1.unlock();
    m2.unlock();
}

TEST_F(LazyTest, testVoid) {
    std::atomic<int> value = 0;
    auto test = [this, &value]() -> Lazy<> {
        CHECK_EXECUTOR(&_executor);
        auto ret = co_await plusOne();
        CHECK_EXECUTOR(&_executor);
        value = ret + 10;
    };
    triggerValue(100);
    syncAwait(test().via(&_executor));
    ASSERT_EQ(111, value.load());
}

TEST_F(LazyTest, testReadyCoro) {
    auto addOne = [](int x) -> Lazy<int> { co_return x + 1; };
    auto solve = [addOne = std::move(addOne)](int x) -> Lazy<int> {
        int tmp = co_await addOne(x);
        co_return 1 + co_await addOne(tmp);
    };
    ASSERT_EQ(10, syncAwait(solve(7)));
}
TEST_F(LazyTest, testExecutor) {
    executors::SimpleExecutor e1(1);
    executors::SimpleExecutor e2(1);
    auto addTwo = [&](int x) -> Lazy<int> {
        CHECK_EXECUTOR(&e2);
        auto tmp = co_await getValue(x);
        CHECK_EXECUTOR(&e2);
        co_return tmp + 2;
    };
    {
        caseTearDown();
        caseSetUp();
        auto test = [&, this]() -> Lazy<int> {
            CHECK_EXECUTOR(&e1);
            int y = co_await plusOne();
            CHECK_EXECUTOR(&e1);
            int z = co_await addTwo(y).via(&e2);
            CHECK_EXECUTOR(&e1);
            co_return z;
        };
        triggerValue(100);
        auto val = syncAwait(test().via(&e1));
        ASSERT_EQ(103, val);
    }
}

TEST_F(LazyTest, testNoCopy) {
    struct NoCopy {
        NoCopy(const NoCopy&) = delete;
        NoCopy& operator=(const NoCopy&) = delete;

        NoCopy() : val(-1) {}
        NoCopy(int v) : val(v) {}
        NoCopy(NoCopy&& other) : val(other.val) { other.val = -1; }
        NoCopy& operator=(NoCopy&& other) {
            std::swap(val, other.val);
            return *this;
        }

        int val;
    };

    auto coro0 = []() -> Lazy<NoCopy> { co_return 10; };
    ASSERT_EQ(10, syncAwait(coro0()).val);
}

TEST_F(LazyTest, testDetachedCoroutine) {
    std::atomic<int> value = 0;
    auto test = [&]() -> Lazy<> {
        CHECK_EXECUTOR(&_executor);
        auto ret = co_await plusOne();
        CHECK_EXECUTOR(&_executor);
        value = ret + 10;
    };
    triggerValue(100);
    test().via(&_executor).start([](Try<void> result) {});
    while (value.load() != 111) {
        std::this_thread::sleep_for(1000us);
    }
}

TEST_F(LazyTest, testCollectAll) {
    executors::SimpleExecutor e1(5);
    executors::SimpleExecutor e2(5);
    executors::SimpleExecutor e3(5);
    auto test = [this, &e1]() -> Lazy<int> {
        std::vector<Lazy<int>> input;
        input.push_back(getValue(1));
        input.push_back(getValue(2));
        CHECK_EXECUTOR(&e1);
        auto combinedLazy = collectAll(std::move(input));
        CHECK_EXECUTOR(&e1);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e1);
        EXPECT_EQ(2u, out.size());
        co_await CurrentExecutor();
        co_return out[0].value() + out[1].value();
    };
    ASSERT_EQ(3, syncAwait(test().via(&e1)));

    auto test1 = [this, &e1]() -> Lazy<> {
        std::vector<Lazy<>> input;
        input.push_back(makeVoidTask());
        input.push_back(makeVoidTask());
        CHECK_EXECUTOR(&e1);
        auto combinedLazy = collectAll(std::move(input));
        CHECK_EXECUTOR(&e1);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e1);
        EXPECT_EQ(2u, out.size());
        co_await CurrentExecutor();
    };
    syncAwait(test1().via(&e1));

    auto test2 = [this, &e1, &e2, &e3]() -> Lazy<int> {
        std::vector<RescheduleLazy<int>> input;
        input.push_back(getValue(1).via(&e1));
        input.push_back(getValue(2).via(&e2));

        CHECK_EXECUTOR(&e3);
        auto combinedLazy = collectAll(std::move(input));
        CHECK_EXECUTOR(&e3);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e3);
        EXPECT_EQ(2u, out.size());
        co_return out[0].value() + out[1].value();
    };
    ASSERT_EQ(3, syncAwait(test2().via(&e3)));

    auto test3 = [this, &e1, &e2, &e3]() -> Lazy<> {
        std::vector<RescheduleLazy<>> input;
        input.push_back(makeVoidTask().via(&e1));
        input.push_back(makeVoidTask().via(&e2));

        CHECK_EXECUTOR(&e3);
        auto combinedLazy = collectAll(std::move(input));
        CHECK_EXECUTOR(&e3);

        auto out = co_await std::move(combinedLazy);
        EXPECT_EQ(2u, out.size());
        CHECK_EXECUTOR(&e3);
    };
    syncAwait(test3().via(&e3));
}

TEST_F(LazyTest, testCollectAllBatched) {
    int task_num = 5000;
    long total = 0;
    for (auto i = 0; i < task_num; i++) {
        total += i;
    }

    executors::SimpleExecutor e1(10);
    executors::SimpleExecutor e2(10);
    executors::SimpleExecutor e3(10);
    executors::SimpleExecutor e4(10);
    executors::SimpleExecutor e5(10);
    executors::SimpleExecutor e6(10);
    // Lazy:
    // collectAllWindowed, maxConcurrency is task_num;
    auto test1 = [this, &e1, &task_num]() -> Lazy<int> {
        std::vector<Lazy<int>> input;
        for (auto i = 0; i < task_num; i++) {
            input.push_back(getValue(i));
        }
        CHECK_EXECUTOR(&e1);
        auto combinedLazy =
            collectAllWindowed(task_num, false, std::move(input));
        CHECK_EXECUTOR(&e1);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e1);
        EXPECT_EQ(task_num, out.size());
        co_await CurrentExecutor();
        int sum = 0;
        for (size_t i = 0; i < out.size(); i++) {
            sum += out[i].value();
        }
        co_return sum;
    };

    {
        ScopeRuntime tt{"Lazy: collectAll_maxConcurrency_is_task_num", 1};
        ASSERT_EQ(total, syncAwait(test1().via(&e1)));
    }

    auto test1Void = [this, &e1, &task_num]() -> Lazy<> {
        std::vector<Lazy<>> input;
        for (auto i = 0; i < 10; i++) {
            input.push_back(makeVoidTask());
        }
        CHECK_EXECUTOR(&e1);
        auto combinedLazy =
            collectAllWindowed(task_num, false, std::move(input));
        CHECK_EXECUTOR(&e1);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e1);
        co_await CurrentExecutor();
    };
    syncAwait(test1Void().via(&e1));

    // Lazy:
    // collectAllWindowed, maxConcurrency is 10;
    auto test2 = [this, &e2, &task_num]() -> Lazy<int> {
        std::vector<Lazy<int>> input;
        for (auto i = 0; i < task_num; i++) {
            input.push_back(getValue(i));
        }
        CHECK_EXECUTOR(&e2);
        auto combinedLazy = collectAllWindowed(10, false, std::move(input));
        CHECK_EXECUTOR(&e2);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e2);
        EXPECT_EQ(task_num, out.size());
        co_await CurrentExecutor();
        int sum = 0;
        for (size_t i = 0; i < out.size(); i++) {
            sum += out[i].value();
        }
        co_return sum;
    };
    {
        ScopeRuntime tt{"Lazy: collectAll_maxConcurrency_is_10", 1};
        ASSERT_EQ(total, syncAwait(test2().via(&e2)));
    }

    auto test2Void = [this, &e2, &task_num]() -> Lazy<> {
        std::vector<Lazy<>> input;
        for (auto i = 0; i < task_num; i++) {
            input.push_back(makeVoidTask());
        }
        CHECK_EXECUTOR(&e2);
        auto combinedLazy = collectAllWindowed(10, false, std::move(input));
        CHECK_EXECUTOR(&e2);

        auto out = co_await std::move(combinedLazy);

        EXPECT_EQ(task_num, out.size());
        CHECK_EXECUTOR(&e2);
        co_await CurrentExecutor();
    };
    syncAwait(test2Void().via(&e2));

    // Lazy:
    // collectAllWindowed, maxConcurrency is 10
    // inAlloc
    std::allocator<Lazy<int>> inAlloc;
    std::allocator<Try<int>> outAlloc;
    auto test3 = [this, &e3, &inAlloc, &task_num]() -> Lazy<int> {
        std::vector<Lazy<int>, std::allocator<Lazy<int>>> input(inAlloc);
        for (auto i = 0; i < task_num; i++) {
            input.push_back(getValue(i));
        }
        CHECK_EXECUTOR(&e3);
        auto combinedLazy = collectAllWindowed(10, false, std::move(input));
        CHECK_EXECUTOR(&e3);
        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e3);
        EXPECT_EQ(task_num, out.size());
        int sum = 0;
        for (size_t i = 0; i < out.size(); i++) {
            sum += out[i].value();
        }
        co_return sum;
    };
    {
        ScopeRuntime tt{"Lazy: collectAll_maxConcurrency_is_10_inAlloc", 1};
        ASSERT_EQ(total, syncAwait(test3().via(&e3)));
    }

    std::allocator<Lazy<>> inAllocVoid;
    auto test3Void = [this, &e3, &inAllocVoid, &task_num]() -> Lazy<> {
        std::vector<Lazy<>, std::allocator<Lazy<>>> input(inAllocVoid);
        for (auto i = 0; i < task_num; i++) {
            input.push_back(makeVoidTask());
        }
        CHECK_EXECUTOR(&e3);
        auto combinedLazy = collectAllWindowed(10, false, std::move(input));
        CHECK_EXECUTOR(&e3);
        auto out = co_await std::move(combinedLazy);

        EXPECT_EQ(task_num, out.size());
        CHECK_EXECUTOR(&e3);
    };
    syncAwait(test3Void().via(&e3));

    // Lazy:
    // collectAllWindowed, maxConcurrency is 10
    // inAlloc && outAlloc
    auto test4 = [this, &e4, &inAlloc, &outAlloc, &task_num]() -> Lazy<int> {
        std::vector<Lazy<int>, std::allocator<Lazy<int>>> input(inAlloc);
        for (auto i = 0; i < task_num; i++) {
            input.push_back(getValue(i));
        }
        CHECK_EXECUTOR(&e4);
        auto combinedLazy =
            collectAllWindowed(10, false, std::move(input), outAlloc);
        CHECK_EXECUTOR(&e4);
        auto out = co_await std::move(combinedLazy);
        EXPECT_EQ(task_num, out.size());
        CHECK_EXECUTOR(&e4);
        int sum = 0;
        for (size_t i = 0; i < out.size(); i++) {
            sum += out[i].value();
        }
        co_return sum;
    };
    {
        ScopeRuntime tt{
            "Lazy: collectAll_maxConcurrency_is_10_inAlloc_outAlloc", 1};
        ASSERT_EQ(total, syncAwait(test4().via(&e4)));
    }

    // RescheduleLazy:
    // collectAllWindowed, maxConcurrency is task_num;
    auto test5 = [this, &e4, &e5, &e6, &task_num]() -> Lazy<int> {
        std::vector<RescheduleLazy<int>> input;
        for (auto i = 0; i < task_num; i++) {
            if (i % 2) {
                input.push_back(getValue(i).via(&e4));
            } else {
                input.push_back(getValue(i).via(&e5));
            }
        }

        CHECK_EXECUTOR(&e6);
        auto combinedLazy =
            collectAllWindowed(task_num, false, std::move(input));
        CHECK_EXECUTOR(&e6);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e6);
        EXPECT_EQ(task_num, out.size());
        int sum = 0;
        for (size_t i = 0; i < out.size(); i++) {
            sum += out[i].value();
        }
        co_return sum;
    };
    {
        ScopeRuntime tt{"RescheduleLazy: collectAll_maxConcurrency_is_task_num",
                        1};
        ASSERT_EQ(total, syncAwait(test5().via(&e6)));
    }

    auto test5Void = [this, &e4, &e5, &e6, &task_num]() -> Lazy<> {
        std::vector<RescheduleLazy<>> input;
        for (auto i = 0; i < task_num; i++) {
            if (i % 2) {
                input.push_back(makeVoidTask().via(&e4));
            } else {
                input.push_back(makeVoidTask().via(&e5));
            }
        }

        CHECK_EXECUTOR(&e6);
        auto combinedLazy =
            collectAllWindowed(task_num, false, std::move(input));
        CHECK_EXECUTOR(&e6);

        auto out = co_await std::move(combinedLazy);

        EXPECT_EQ(task_num, out.size());
        CHECK_EXECUTOR(&e6);
    };
    syncAwait(test5Void().via(&e6));

    // RescheduleLazy:
    // collectAllWindowed, maxConcurrency is 10;
    auto test6 = [this, &e4, &e5, &e6, &task_num]() -> Lazy<int> {
        std::vector<RescheduleLazy<int>> input;
        for (auto i = 0; i < task_num; i++) {
            if (i % 2) {
                input.push_back(getValue(i).via(&e4));
            } else {
                input.push_back(getValue(i).via(&e5));
            }
        }

        CHECK_EXECUTOR(&e6);
        auto combinedLazy = collectAllWindowed(10, false, std::move(input));
        CHECK_EXECUTOR(&e6);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e6);
        EXPECT_EQ(task_num, out.size());
        int sum = 0;
        for (size_t i = 0; i < out.size(); i++) {
            sum += out[i].value();
        }
        co_return sum;
    };
    {
        ScopeRuntime tt{"RescheduleLazy: collectAll_maxConcurrency_is_10", 1};
        ASSERT_EQ(total, syncAwait(test6().via(&e6)));
    }

    auto test6Void = [this, &e4, &e5, &e6, &task_num]() -> Lazy<> {
        std::vector<RescheduleLazy<>> input;
        for (auto i = 0; i < task_num; i++) {
            if (i % 2) {
                input.push_back(makeVoidTask().via(&e4));
            } else {
                input.push_back(makeVoidTask().via(&e5));
            }
        }

        CHECK_EXECUTOR(&e6);
        auto combinedLazy = collectAllWindowed(10, false, std::move(input));
        CHECK_EXECUTOR(&e6);
        auto out = co_await std::move(combinedLazy);
        EXPECT_EQ(task_num, out.size());
        CHECK_EXECUTOR(&e6);
    };
    syncAwait(test6Void().via(&e6));

    // RescheduleLazy:
    // collectAllWindowed, maxConcurrency is 10;
    // inAlloc1
    std::allocator<RescheduleLazy<int>> inAlloc1;
    std::allocator<Try<int>> outAlloc1;
    auto test7 = [this, &e4, &e5, &e6, &inAlloc1, &task_num]() -> Lazy<int> {
        std::vector<RescheduleLazy<int>, std::allocator<RescheduleLazy<int>>>
            input(inAlloc1);
        for (auto i = 0; i < task_num; i++) {
            if (i % 2) {
                input.push_back(getValue(i).via(&e4));
            } else {
                input.push_back(getValue(i).via(&e5));
            }
        }

        CHECK_EXECUTOR(&e6);
        auto combinedLazy = collectAllWindowed(10, false, std::move(input));
        CHECK_EXECUTOR(&e6);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e6);
        EXPECT_EQ(task_num, out.size());
        int sum = 0;
        for (size_t i = 0; i < out.size(); i++) {
            sum += out[i].value();
        }
        co_return sum;
    };
    {
        ScopeRuntime tt{
            "RescheduleLazy: collectAll_maxConcurrency_is_10_inAlloc", 1};
        ASSERT_EQ(total, syncAwait(test7().via(&e6)));
    }

    std::allocator<RescheduleLazy<>> inAlloc1Void;
    auto test7Void = [this, &e4, &e5, &e6, &inAlloc1Void,
                      &task_num]() -> Lazy<> {
        std::vector<RescheduleLazy<>, std::allocator<RescheduleLazy<>>> input(
            inAlloc1Void);
        for (auto i = 0; i < task_num; i++) {
            if (i % 2) {
                input.push_back(makeVoidTask().via(&e4));
            } else {
                input.push_back(makeVoidTask().via(&e5));
            }
        }

        CHECK_EXECUTOR(&e6);
        auto combinedLazy = collectAllWindowed(10, false, std::move(input));
        CHECK_EXECUTOR(&e6);
        auto out = co_await std::move(combinedLazy);

        EXPECT_EQ(task_num, out.size());
        CHECK_EXECUTOR(&e6);
    };
    syncAwait(test7Void().via(&e6));

    // RescheduleLazy:
    // collectAllWindowed, maxConcurrency is 10;
    // inAlloc1, outAlloc1
    auto test8 = [this, &e4, &e5, &e6, inAlloc1, outAlloc1,
                  &task_num]() -> Lazy<int> {
        std::vector<RescheduleLazy<int>, std::allocator<RescheduleLazy<int>>>
            input(inAlloc1);
        for (auto i = 0; i < task_num; i++) {
            if (i % 2) {
                input.push_back(getValue(i).via(&e4));
            } else {
                input.push_back(getValue(i).via(&e5));
            }
        }

        CHECK_EXECUTOR(&e6);
        auto combinedLazy =
            collectAllWindowed(10, false, std::move(input), outAlloc1);
        CHECK_EXECUTOR(&e6);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e6);
        EXPECT_EQ(task_num, out.size());
        int sum = 0;
        for (size_t i = 0; i < out.size(); i++) {
            sum += out[i].value();
        }
        co_return sum;
    };
    {
        ScopeRuntime tt{
            "RescheduleLazy: collectAll_maxConcurrency_is_10_inAlloc_outAlloc",
            1};
        ASSERT_EQ(total, syncAwait(test8().via(&e6)));
    }
}

TEST_F(LazyTest, testCollectAllWithAllocator) {
    executors::SimpleExecutor e1(5);
    executors::SimpleExecutor e2(5);
    executors::SimpleExecutor e3(5);
    std::allocator<Lazy<int>> inAlloc;
    std::allocator<Try<int>> outAlloc;
    auto test0 = [this, &e1, &inAlloc]() -> Lazy<int> {
        std::vector<Lazy<int>, std::allocator<Lazy<int>>> input(inAlloc);
        input.push_back(getValue(1));
        input.push_back(getValue(2));
        CHECK_EXECUTOR(&e1);
        auto combinedLazy = collectAll(std::move(input));
        CHECK_EXECUTOR(&e1);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e1);
        EXPECT_EQ(2u, out.size());
        co_return out[0].value() + out[1].value();
    };
    ASSERT_EQ(3, syncAwait(test0().via(&e1)));

    // FIXME
    auto test1 = [this, &e1, &inAlloc, &outAlloc]() -> Lazy<int> {
        std::vector<Lazy<int>, std::allocator<Lazy<int>>> input(inAlloc);
        input.push_back(getValue(1));
        input.push_back(getValue(2));
        CHECK_EXECUTOR(&e1);
        auto combinedLazy = collectAll(std::move(input), outAlloc);
        CHECK_EXECUTOR(&e1);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e1);
        EXPECT_EQ(2u, out.size());
        co_return out[0].value() + out[1].value();
    };
    ASSERT_EQ(3, syncAwait(test1().via(&e1)));

    auto test2 = [this, &e1, &e2, &e3]() -> Lazy<int> {
        std::vector<RescheduleLazy<int>> input;
        input.push_back(getValue(1).via(&e1));
        input.push_back(getValue(2).via(&e2));

        CHECK_EXECUTOR(&e3);
        auto combinedLazy = collectAll(std::move(input));
        CHECK_EXECUTOR(&e3);

        auto out = co_await std::move(combinedLazy);

        CHECK_EXECUTOR(&e3);
        EXPECT_EQ(2u, out.size());
        co_return out[0].value() + out[1].value();
    };
    ASSERT_EQ(3, syncAwait(test2().via(&e3)));
}

TEST_F(LazyTest, testCollectAllVariadic) {
    // normal task
    executors::SimpleExecutor e1(5);
    auto test = [this, &e1]() -> Lazy<> {
        Lazy<int> intLazy = getValue(2);
        Lazy<double> doubleLazy = getValue(2.2);
        Lazy<std::string> stringLazy =
            getValue(std::string("testCollectAllVariadic"));

        CHECK_EXECUTOR(&e1);
        auto combinedLazy = collectAll(
            std::move(intLazy), std::move(doubleLazy), std::move(stringLazy));
        CHECK_EXECUTOR(&e1);

        auto out_tuple = co_await std::move(combinedLazy);
        EXPECT_EQ(3u, std::tuple_size<decltype(out_tuple)>::value);

        auto v_try_int = std::get<0>(std::move(out_tuple));
        auto v_try_double = std::get<1>(std::move(out_tuple));
        auto v_try_string = std::get<2>(std::move(out_tuple));

        EXPECT_EQ(2, v_try_int.value());
        EXPECT_DOUBLE_EQ(2.2, v_try_double.value());
        EXPECT_STREQ("testCollectAllVariadic", v_try_string.value().c_str());

        CHECK_EXECUTOR(&e1);
    };
    syncAwait(test().via(&e1));

    // void task
    executors::SimpleExecutor e2(5);
    auto test1 = [this, &e2]() -> Lazy<> {
        Lazy<int> intLazy = getValue(2);
        Lazy<void> voidLazy01 = makeVoidTask();
        Lazy<void> voidLazy02 = testExcept();

        CHECK_EXECUTOR(&e2);
        auto combinedLazy = collectAll(
            std::move(intLazy), std::move(voidLazy01), std::move(voidLazy02));
        CHECK_EXECUTOR(&e2);

        auto out_tuple = co_await std::move(combinedLazy);
        EXPECT_EQ(3u, std::tuple_size<decltype(out_tuple)>::value);

        auto v_try_int = std::get<0>(std::move(out_tuple));
        auto v_try_void01 = std::get<1>(std::move(out_tuple));
        auto v_try_void02 = std::get<2>(std::move(out_tuple));

        EXPECT_EQ(2u, v_try_int.value());

        bool b1 = std::is_same_v<Try<void>, decltype(v_try_void01)>;
        bool b2 = std::is_same_v<Try<void>, decltype(v_try_void02)>;
        try {  // v_try_void02 throw exception
            EXPECT_TRUE(true);
            v_try_void02.value();
            EXPECT_TRUE(false);
        } catch (const std::runtime_error& e) {
            EXPECT_TRUE(true);
        }
        EXPECT_TRUE(b1);
        EXPECT_TRUE(b2);

        CHECK_EXECUTOR(&e2);
    };
    syncAwait(test1().via(&e2));

    // RescheduleLazy
    executors::SimpleExecutor e3(5);
    executors::SimpleExecutor e4(5);
    auto test2 = [this, &e1, &e2, &e3, &e4]() -> Lazy<> {
        RescheduleLazy<int> intLazy = getValue(2).via(&e2);
        RescheduleLazy<double> doubleLazy = getValue(2.2).via(&e3);
        RescheduleLazy<std::string> stringLazy =
            getValue(std::string("testCollectAllVariadic")).via(&e4);

        CHECK_EXECUTOR(&e1);
        auto combinedLazy = collectAll(
            std::move(intLazy), std::move(doubleLazy), std::move(stringLazy));
        CHECK_EXECUTOR(&e1);

        auto out_tuple = co_await std::move(combinedLazy);
        EXPECT_EQ(3u, std::tuple_size<decltype(out_tuple)>::value);

        auto v_try_int = std::get<0>(std::move(out_tuple));
        auto v_try_double = std::get<1>(std::move(out_tuple));
        auto v_try_string = std::get<2>(std::move(out_tuple));

        EXPECT_EQ(2, v_try_int.value());
        EXPECT_DOUBLE_EQ(2.2, v_try_double.value());
        EXPECT_STREQ("testCollectAllVariadic", v_try_string.value().c_str());

        CHECK_EXECUTOR(&e1);
    };
    syncAwait(test2().via(&e1));

    // void RescheduleLazy
    auto test3 = [this, &e1, &e2, &e3, &e4]() -> Lazy<> {
        RescheduleLazy<int> intLazy = getValue(2).via(&e2);
        RescheduleLazy<void> voidLazy01 = makeVoidTask().via(&e3);
        RescheduleLazy<void> voidLazy02 = makeVoidTask().via(&e4);

        CHECK_EXECUTOR(&e1);
        auto combinedLazy = collectAll(
            std::move(intLazy), std::move(voidLazy01), std::move(voidLazy02));
        CHECK_EXECUTOR(&e1);

        auto out_tuple = co_await std::move(combinedLazy);
        EXPECT_EQ(3u, std::tuple_size<decltype(out_tuple)>::value);

        auto v_try_int = std::get<0>(std::move(out_tuple));
        auto v_try_void01 = std::get<1>(std::move(out_tuple));
        auto v_try_void02 = std::get<2>(std::move(out_tuple));

        EXPECT_EQ(2u, v_try_int.value());
        bool b1 = std::is_same_v<Try<void>, decltype(v_try_void01)>;
        bool b2 = std::is_same_v<Try<void>, decltype(v_try_void02)>;
        EXPECT_TRUE(b1);
        EXPECT_TRUE(b2);

        CHECK_EXECUTOR(&e1);
    };
    syncAwait(test3().via(&e1));
}

TEST_F(LazyTest, testCollectAny) {
    srand((unsigned)time(NULL));
    executors::SimpleExecutor e1(10);
    executors::SimpleExecutor e2(10);
    executors::SimpleExecutor e3(10);

    auto test = [this]() -> Lazy<int> {
        std::vector<Lazy<int>> input;
        input.push_back(getValueWithSleep(1));
        input.push_back(getValueWithSleep(2));
        input.push_back(getValueWithSleep(2));
        input.push_back(getValueWithSleep(3));
        input.push_back(getValueWithSleep(4));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(3));
        input.push_back(getValueWithSleep(4));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        input.push_back(getValueWithSleep(5));
        auto combinedLazy = collectAny(std::move(input));
        auto out = co_await std::move(combinedLazy);
        EXPECT_GT(out._value.value(), 0);
        EXPECT_GE(out._idx, 0);
        co_return out._value.value();
    };
    ASSERT_GT(syncAwait(test().via(&e1)), 0);

    auto test2 = [this, &e1, &e2, &e3]() -> Lazy<int> {
        std::vector<RescheduleLazy<int>> input;
        input.push_back(getValueWithSleep(11).via(&e1));
        input.push_back(getValueWithSleep(12).via(&e1));
        input.push_back(getValueWithSleep(13).via(&e1));
        input.push_back(getValueWithSleep(14).via(&e1));
        input.push_back(getValueWithSleep(15).via(&e1));

        input.push_back(getValueWithSleep(25).via(&e2));
        input.push_back(getValueWithSleep(21).via(&e2));
        input.push_back(getValueWithSleep(22).via(&e2));
        input.push_back(getValueWithSleep(23).via(&e2));
        input.push_back(getValueWithSleep(24).via(&e2));
        input.push_back(getValueWithSleep(25).via(&e2));

        CHECK_EXECUTOR(&e3);
        auto combinedLazy = collectAny(std::move(input));
        CHECK_EXECUTOR(&e3);

        auto out = co_await std::move(combinedLazy);

        EXPECT_GT(out._value.value(), 10);
        EXPECT_GE(out._idx, 0);
        co_return out._value.value();
    };
    ASSERT_GT(syncAwait(test2().via(&e3)), 10);

    std::this_thread::sleep_for(chrono::seconds(2));
}

TEST_F(LazyTest, testException) {
    executors::SimpleExecutor e1(1);
    int ret = 0;
    auto test0 = [&]() -> Lazy<> {
        throw std::runtime_error("error test0");
        co_return;
    };
    auto test1 = [&]() -> Lazy<int> {
        throw std::runtime_error("error test1");
        co_return 0;
    };

    auto test2 = [&]() mutable -> Lazy<> {
        try {
            co_await test0();
            ret += 1;
        } catch (const std::runtime_error& e) {
            ;
        }
        try {
            co_await test1();
            ret += 1;
        } catch (const std::runtime_error& e) {
            ;
        }
    };
    syncAwait(test2().via(&e1));
    ASSERT_EQ(0, ret);
}

TEST_F(LazyTest, testContext) {
    executors::SimpleExecutor e1(10);
    executors::SimpleExecutor e2(10);

    auto addTwo = [&](int x) -> Lazy<int> {
        CHECK_EXECUTOR(&e2);
        auto tid = std::this_thread::get_id();
        auto tmp = co_await getValue(x);
        CHECK_EXECUTOR(&e2);
        EXPECT_EQ(tid, std::this_thread::get_id());
        co_return tmp + 2;
    };
    {
        caseTearDown();
        caseSetUp();
        auto test = [&, this]() -> Lazy<int> {
            CHECK_EXECUTOR(&e1);
            auto tid = std::this_thread::get_id();
            int y = co_await plusOne();
            CHECK_EXECUTOR(&e1);
            EXPECT_EQ(tid, std::this_thread::get_id());
            int z = co_await addTwo(y).via(&e2);
            CHECK_EXECUTOR(&e1);
            EXPECT_EQ(tid, std::this_thread::get_id());
            co_return z;
        };
        triggerValue(100);
        auto val = syncAwait(test().via(&e1));
        ASSERT_EQ(103, val);
    }
}

struct A {
    int* a;
    A(int x) : a(new int(x)) {}
    ~A() {
        if (a) {
            destroyOrder.push_back(*a);
            delete a;
        }
    }
    int show() { return *a; }
    A(A&& other) : a(other.a) { other.a = nullptr; }

    static std::vector<int> destroyOrder;
};

std::vector<int> A::destroyOrder;

Lazy<int> getValue(A x) {
    struct ValueAwaiter {
        int value;
        ValueAwaiter(int v) : value(v) {}

        bool await_ready() { return false; }
        void await_suspend(STD_CORO::coroutine_handle<> continuation) noexcept {
            std::thread([c = std::move(continuation)]() mutable {
                c.resume();
            }).detach();
        }
        int await_resume() noexcept { return value; }
    };
    auto z = co_await ValueAwaiter(x.show());

    co_return z + x.show();
}

Lazy<int> f1(A a) {
    auto v = co_await getValue(1);
    std::vector<Lazy<int>> lzs;
    lzs.push_back(getValue(7));
    auto r = co_await collectAll(std::move(lzs));
    co_return a.show() + v + r[0].value();
};

Lazy<int> f0(A a) {
    int v = 0;
    v = co_await f1(1);
    cout << a.show() << endl;
    co_return v;
}

TEST_F(LazyTest, testDestroyOrder) {
    A::destroyOrder.clear();
    auto test = []() -> Lazy<int> {
        auto a = new A(999);
        auto l = f0(0);
        auto v = co_await l;
        delete a;
        co_return v;
    };
    syncAwait(test().via(&_executor));
    EXPECT_THAT(A::destroyOrder, ElementsAre(1, 7, 1, 0, 999));
}

namespace detail {
template <int N>
Lazy<int> lazy_fn() {
    co_return N + co_await lazy_fn<N - 1>();
}

template <>
Lazy<int> lazy_fn<0>() {
    co_return 0;
}
}  // namespace detail
TEST_F(LazyTest, testLazyPerf) {
    auto test_loop = 5000;
    auto total = 0;

    auto one = [](int n) -> Lazy<int> {
        auto x = n;
        co_return x;
    };

    auto loop_starter = [&]() -> Lazy<> {
        ScopeRuntime scoper("lazy 30 loop call", test_loop);
        for (auto k = 0; k < test_loop; ++k) {
            for (auto i = 1; i <= 30; ++i) {
                total += co_await one(i);
            }
        }
    };
    syncAwait(loop_starter());
    ASSERT_EQ(total, 2325000);

    total = 0;
    auto chain_starter = [&]() -> Lazy<> {
        ScopeRuntime scoper("lazy 30 chain call", test_loop);
        for (auto i = 0; i < test_loop; ++i) {
            total += co_await detail::lazy_fn<30>();
        }
    };
    syncAwait(chain_starter());
    ASSERT_EQ(total, 2325000);
}

TEST_F(LazyTest, testcollectAllParallel) {
    executors::SimpleExecutor e1(10);
    auto test1 = [this]() -> Lazy<> {
        std::vector<Lazy<std::thread::id>> input;
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        auto combinedLazy = collectAll(std::move(input));
        auto out = co_await std::move(combinedLazy);
        EXPECT_EQ(out[0].value(), out[1].value());
        EXPECT_EQ(out[0].value(), out[2].value());
        EXPECT_EQ(out[0].value(), out[3].value());
        EXPECT_EQ(out[0].value(), out[4].value());
        EXPECT_EQ(out[0].value(), out[5].value());
    };
    syncAwait(test1().via(&e1));

    auto test2 = [this]() -> Lazy<> {
        std::vector<Lazy<std::thread::id>> input;
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        auto combinedLazy = collectAllPara(std::move(input));
        auto out = co_await std::move(combinedLazy);
        std::cout << out[0].value() << std::endl;
        std::cout << out[1].value() << std::endl;
        std::cout << out[2].value() << std::endl;
        std::cout << out[3].value() << std::endl;
        std::cout << out[4].value() << std::endl;
        std::cout << out[5].value() << std::endl;
        std::cout << out[6].value() << std::endl;
        std::cout << out[7].value() << std::endl;
        std::set<std::thread::id> ss;
        ss.insert(out[0].value());
        ss.insert(out[1].value());
        ss.insert(out[2].value());
        ss.insert(out[3].value());
        ss.insert(out[4].value());
        ss.insert(out[5].value());
        ss.insert(out[6].value());
        ss.insert(out[7].value());
        // FIXME: input tasks maybe run not in different thread.
        EXPECT_GT(ss.size(), 2);
    };
    syncAwait(test2().via(&e1));
}

std::vector<int> result;
Lazy<void> makeTest(int value) {
    std::this_thread::sleep_for(1000us);
    result.push_back(value);
    co_return;
}

TEST_F(LazyTest, testBatchedcollectAll) {
    executors::SimpleExecutor e1(10);
    auto test1 = [this]() -> Lazy<> {
        std::vector<Lazy<std::thread::id>> input;
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        input.push_back(getThreadId());
        auto combinedLazy = collectAllWindowed(
            2 /*maxConcurrency*/, false /*yield*/, std::move(input));
        auto out = co_await std::move(combinedLazy);
        std::cout
            << "input tasks maybe run not in different thread, thread id: "
            << std::endl;
        std::cout << out[0].value() << std::endl;
        std::cout << out[1].value() << std::endl;
        std::cout << out[2].value() << std::endl;
        std::cout << out[3].value() << std::endl;
        std::cout << out[4].value() << std::endl;
        std::cout << out[5].value() << std::endl;
        std::cout << out[6].value() << std::endl;
        std::set<std::thread::id> ss;
        ss.insert(out[0].value());
        ss.insert(out[1].value());
        ss.insert(out[2].value());
        ss.insert(out[3].value());
        ss.insert(out[4].value());
        ss.insert(out[5].value());
        ss.insert(out[6].value());
        // FIXME: input tasks maybe run not in different thread.
        EXPECT_GT(ss.size(), 1);
    };
    syncAwait(test1().via(&e1));

    // test Yield
    executors::SimpleExecutor e2(1);
    int value1 = 1;
    int value2 = 2;
    int value3 = 3;
    int value4 = 4;

    int value5 = 5;
    int value6 = 6;
    int value7 = 7;
    int value8 = 8;

    std::vector<Lazy<void>> input1;
    input1.push_back(makeTest(value1));
    input1.push_back(makeTest(value2));
    input1.push_back(makeTest(value3));
    input1.push_back(makeTest(value4));

    std::vector<Lazy<void>> input2;
    input2.push_back(makeTest(value5));
    input2.push_back(makeTest(value6));
    input2.push_back(makeTest(value7));
    input2.push_back(makeTest(value8));

    collectAllWindowed(1, true, std::move(input1))
        .via(&e2)
        .start([](auto result) {});
    collectAllWindowed(1, true, std::move(input2))
        .via(&e2)
        .start([](auto result) {});
    std::this_thread::sleep_for(500000us);
    std::vector<int> expect{1, 5, 2, 6, 3, 7, 4, 8};
    for (size_t i = 0; i < result.size(); ++i) {
        EXPECT_EQ(expect[i], result[i]);
        std::cout << "expect[" << i << "]: " << expect[i] << ", "
                  << "result[" << i << "]: " << result[i] << std::endl;
    }
}

TEST_F(LazyTest, testDetach) {
    util::Condition cond;
    int count = 0;
    executors::SimpleExecutor e1(1);
    auto test1 = [&]() -> Lazy<int> {
        count += 2;
        cond.release();
        co_return count;
    };
    test1().via(&e1).detach();
    cond.acquire();
    EXPECT_EQ(count, 2);
}

}  // namespace coro
}  // namespace async_simple
