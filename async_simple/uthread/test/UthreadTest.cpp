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
#include <async_simple/Common.h>
#include <async_simple/coro/Lazy.h>
#include <async_simple/executors/SimpleExecutor.h>
#include <async_simple/test/unittest.h>
#include <async_simple/uthread/Async.h>
#include <async_simple/uthread/Await.h>
#include <async_simple/uthread/Collect.h>
#include <async_simple/uthread/Latch.h>
#include <async_simple/uthread/Uthread.h>
#include <exception>
#include <functional>
#include <iostream>
#include <type_traits>

using namespace std;

namespace async_simple {
namespace uthread {

class UthreadTest : public FUTURE_TESTBASE {
public:
    UthreadTest() : _executor(4) {}
    void caseSetUp() override {}
    void caseTearDown() override {}

    template <class Func>
    void delayedTask(Func&& func, std::size_t ms) {
        std::thread(
            [f = std::move(func), ms](Executor* ex) {
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                ex->schedule(std::move(f));
            },
            &_executor)
            .detach();
    }

    template <class T>
    struct Awaiter {
        Executor* ex;
        T value;

        Awaiter(Executor* e, T v) : ex(e), value(v) {}

        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> handle) noexcept {
            auto ctx = ex->checkout();
            std::thread([handle, e = ex, ctx]() mutable {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                Executor::Func f = [handle]() mutable { handle.resume(); };
                e->checkin(std::move(f), ctx);
            }).detach();
        }
        T await_resume() noexcept { return value; }
    };
    template <class T>
    coro::Lazy<T> lazySum(T x, T y) {
        co_return co_await Awaiter{&_executor, x + y};
    }

protected:
    executors::SimpleExecutor _executor;
};

TEST_F(UthreadTest, testSimple) {
    auto show = [&](const std::string& message) mutable {
        std::cout << message << "\n";
    };
    std::atomic<bool> done(false);
    _executor.schedule([&done, ex = &_executor, &show]() {
        Uthread ut(Attribute{ex}, [&show]() { show("task 1"); });
        ut.join([ex, &done, &show]() {
            show("task 1 done");
            ex->schedule([ex, &done, &show]() {
                Uthread ut(Attribute{ex}, [&show]() { show("task 2"); });
                ut.join([&done, &show]() {
                    show("task 2 done");
                    done = true;
                });
            });
        });
    });
    while (!done) {
    }
}

TEST_F(UthreadTest, testSwitch) {
    Executor* ex = &_executor;
    auto show = [&](const std::string& message) mutable {
        std::cout << message << "\n";
    };

    auto ioJob = [&]() -> Future<int> {
        Promise<int> p;
        auto f = p.getFuture().via(&_executor);
        delayedTask(
            [p = std::move(p)]() mutable {
                auto value = 1024;
                p.setValue(value);
            },
            100);
        return f;
    };

    std::atomic<int> running = 2;
    _executor.schedule([ex, &running, &show, &ioJob]() mutable {
        Uthread task1(Attribute{ex}, [&running, &show, &ioJob]() {
            show("task1 start");
            auto value = await(ioJob());
            EXPECT_EQ(1024, value);
            show("task1 done");
            running--;
        });
        task1.detach();
    });
    _executor.schedule([ex, &running, &show]() mutable {
        Uthread task2(Attribute{ex}, [&running, &show]() {
            show("task2 start");
            show("task2 done");
            running--;
        });
        task2.detach();
    });

    while (running) {
    }
}

class FakeExecutor : public executors::SimpleExecutor {
public:
    FakeExecutor(size_t threadNum) : SimpleExecutor(threadNum) {}
    ~FakeExecutor() {}

public:
    bool currentThreadInExecutor() const override { return true; }
    Context checkout() override { return NULLCTX; }
    bool checkin(Func func, Context ctx, ScheduleOptions opts) override {
        return schedule(std::move(func));
    }
};

// reschedule uthread to two different executor is not thread-safe
// this case used to check uthread's continuation switched in a new thread
// successfully. detail see: jump_buf_link::switch_in
TEST_F(UthreadTest, testScheduleInTwoThread) {
    auto ex = std::make_unique<executors::SimpleExecutor>(1);
    FakeExecutor fakeEx(1);
    auto show = [&](const std::string& message) mutable {
        std::cout << message << std::endl;
    };

    auto ioJob = [&]() -> Future<int> {
        Promise<int> p;
        auto f = p.getFuture().via(&fakeEx);
        delayedTask(
            [p = std::move(p), &ex]() mutable {
                auto value = 1024;
                // wait task done, avoid data race
                ex.reset();
                p.setValue(value);
            },
            1000);
        return f;
    };

    std::atomic<int> running = 1;
    ex->schedule([ex = &fakeEx, &running, &show, &ioJob]() mutable {
        Uthread task(Attribute{ex}, [&running, &show, &ioJob]() {
            show("task start");
            auto value = await(ioJob());
            EXPECT_EQ(1024, value);
            show("task done");
            running--;
        });
        task.detach();
    });

    while (running) {
    }
}

TEST_F(UthreadTest, testAsync) {
    Executor* ex = &_executor;
    auto show = [&](const std::string& message) mutable {
        std::cout << message << "\n";
    };

    auto ioJob = [&]() -> Future<int> {
        Promise<int> p;
        auto f = p.getFuture().via(&_executor);
        delayedTask(
            [p = std::move(p)]() mutable {
                auto value = 1024;
                p.setValue(value);
            },
            100);
        return f;
    };

    std::atomic<int> running = 2;
    async<Launch::Schedule>(
        [&running, &show, &ioJob]() {
            show("task1 start");
            auto value = await(ioJob());
            EXPECT_EQ(1024, value);
            show("task1 done");
            running--;
        },
        ex);
    async<Launch::Schedule>(
        [&running, &show, ex]() {
            show("task2 start");
            async<Launch::Prompt>([&show]() { show("task3"); }, ex).detach();
            show("task2 done");
            running--;
        },
        ex);

    while (running) {
    }
}

TEST_F(UthreadTest, testAwait) {
    Executor* ex = &_executor;
    auto show = [&](const std::string& message) mutable {
        std::cout << message << "\n";
    };

    auto ioJob = [&](Promise<int> p) {
        delayedTask(
            [p = std::move(p)]() mutable {
                auto value = 1024;
                p.setValue(value);
            },
            100);
    };

    std::atomic<int> running = 2;
    async<Launch::Schedule>(
        [&running, &show, &ioJob, ex]() {
            show("task1 start");
            auto value = await<int>(ex, ioJob);
            EXPECT_EQ(1024, value);
            show("task1 done");
            running--;
        },
        ex);
    async<Launch::Schedule>(
        [&running, &show, ex]() {
            show("task2 start");
            async<Launch::Prompt>([&show]() { show("task3"); }, ex).detach();
            show("task2 done");
            running--;
        },
        ex);

    while (running) {
    }
}

TEST_F(UthreadTest, testAwaitCoroutine) {
    Executor* ex = &_executor;
    auto show = [&](const std::string& message) mutable {
        std::cout << message << "\n";
    };

    std::atomic<int> running = 2;
    async<Launch::Schedule>(
        [&running, &show, ex, this]() mutable {
            show("task1 start");
            auto value =
                await(ex, &std::remove_pointer_t<decltype(this)>::lazySum<int>,
                      this, 1000, 24);
            EXPECT_EQ(1024, value);
            show("task1 done");
            running--;
        },
        ex);
    async<Launch::Schedule>(
        [&running, &show, ex]() {
            show("task2 start");
            async<Launch::Prompt>([&show]() { show("task3"); }, ex).detach();
            show("task2 done");
            running--;
        },
        ex);

    while (running) {
    }
}

namespace globalfn {

template <class T>
struct Awaiter {
    Executor* ex;
    T value;

    Awaiter(Executor* e, T v) : ex(e), value(v) {}

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> handle) noexcept {
        auto ctx = ex->checkout();
        std::thread([handle, e = ex, ctx]() mutable {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            Executor::Func f = [handle]() mutable { handle.resume(); };
            e->checkin(std::move(f), ctx);
        }).detach();
    }
    T await_resume() noexcept { return value; }
};
template <class T>
coro::Lazy<T> lazySum(Executor* ex, T x, T y) {
    co_return co_await Awaiter{ex, x + y};
}

}  // namespace globalfn

TEST_F(UthreadTest, testAwaitCoroutineNoneMemFn) {
    Executor* ex = &_executor;
    auto show = [&](const std::string& message) mutable {
        std::cout << message << "\n";
    };

    std::atomic<int> running = 2;
    async<Launch::Schedule>(
        [&running, &show, ex]() mutable {
            show("task1 start");
            auto value = await(ex, globalfn::lazySum<int>, ex, 1000, 24);
            EXPECT_EQ(1024, value);
            show("task1 done");
            running--;
        },
        ex);

    auto lazySumWrapper = [ex = &_executor]() -> coro::Lazy<int> {
        co_return co_await globalfn::lazySum(ex, 1000, 24);
    };
    async<Launch::Schedule>(
        [&running, &show, ex, &lazySumWrapper]() mutable {
            show("task2 start");
            auto value = await(ex, lazySumWrapper);
            EXPECT_EQ(1024, value);
            show("task2 done");
            running--;
        },
        ex);

    while (running) {
    }
}

TEST_F(UthreadTest, testCollectAllSimple) {
    static constexpr std::size_t kMaxTask = 10;

    Executor* ex = &_executor;
    std::atomic<std::size_t> n = kMaxTask;
    std::vector<std::function<void()>> fs;
    for (size_t i = 0; i < kMaxTask; ++i) {
        fs.emplace_back([i, &n]() {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(kMaxTask - i));
            n--;
        });
    }

    std::atomic<int> running = 1;
    async<Launch::Schedule>(
        [&running, &n, ex, fs = std::move(fs)]() mutable {
            collectAll<Launch::Schedule>(fs.begin(), fs.end(), ex);
            EXPECT_EQ(0u, n);
            running--;
        },
        ex);

    while (running) {
    }
}

TEST_F(UthreadTest, testCollectAllSlow) {
    static constexpr std::size_t kMaxTask = 10;
    Executor* ex = &_executor;

    auto ioJob = [&](std::size_t delay_ms) -> Future<int> {
        Promise<int> p;
        auto f = p.getFuture().via(ex);
        delayedTask(
            [p = std::move(p)]() mutable {
                auto value = 1024;
                p.setValue(value);
            },
            delay_ms);
        return f;
    };

    std::vector<std::function<std::size_t()>> fs;
    for (size_t i = 0; i < kMaxTask; ++i) {
        fs.emplace_back([i, &ioJob]() -> std::size_t {
            return i + await(ioJob(kMaxTask - i));
        });
    }

    std::atomic<int> running = 1;
    async<Launch::Schedule>(
        [&running, ex, fs = std::move(fs)]() mutable {
            auto res = collectAll<Launch::Schedule>(fs.begin(), fs.end(), ex);
            EXPECT_EQ(kMaxTask, res.size());
            for (size_t i = 0; i < kMaxTask; ++i) {
                EXPECT_EQ(i + 1024, res[i]);
            }
            running--;
        },
        ex);

    while (running) {
    }
}

TEST_F(UthreadTest, testCollectAllSlowSingleThread) {
    static constexpr std::size_t kMaxTask = 10;
    Executor* ex = &_executor;

    auto ioJob = [&](std::size_t delay_ms) -> Future<int> {
        Promise<int> p;
        auto f = p.getFuture().via(ex);
        delayedTask(
            [p = std::move(p)]() mutable {
                auto value = 1024;
                p.setValue(value);
            },
            delay_ms);
        return f;
    };

    std::vector<std::function<std::size_t()>> fs;
    for (size_t i = 0; i < kMaxTask; ++i) {
        fs.emplace_back([i, &ioJob]() -> std::size_t {
            return i + await(ioJob(kMaxTask - i));
        });
    }

    std::atomic<int> running = 1;
    async<Launch::Schedule>(
        [&running, ex, fs = std::move(fs)]() mutable {
            auto res = collectAll<Launch::Current>(fs.begin(), fs.end(), ex);
            EXPECT_EQ(kMaxTask, res.size());
            for (size_t i = 0; i < kMaxTask; ++i) {
                EXPECT_EQ(i + 1024, res[i]);
            }
            running--;
        },
        ex);

    while (running) {
    }
}

TEST_F(UthreadTest, testLatch) {
    static constexpr std::size_t kMaxTask = 10;
    Executor* ex = &_executor;

    Latch latch(kMaxTask);
    std::vector<std::function<void()>> fs;
    for (size_t i = 0; i < kMaxTask; ++i) {
        fs.emplace_back([i, latchPtr = &latch]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(i));
            latchPtr->downCount();
        });
    }

    std::atomic<int> running = 1;
    async<Launch::Schedule>(
        [&running, ex, fs = std::move(fs), latchPtr = &latch]() mutable {
            for (size_t i = 0; i < kMaxTask; ++i) {
                async<Launch::Schedule>(std::move(fs[i]), ex);
            }
            latchPtr->await(ex);
            EXPECT_EQ(0u, latchPtr->currentCount());
            running--;
        },
        ex);

    while (running) {
    }
}

TEST_F(UthreadTest, testLatchThreadSafe) {
    static constexpr std::size_t kMaxTask = 1000;
    std::atomic<std::size_t> runningTask(kMaxTask);
    executors::SimpleExecutor taskEx(6);
    executors::SimpleExecutor taskNotify(8);

    for (size_t i = 0; i < kMaxTask; ++i) {
        async<Launch::Schedule>(
            [&taskEx, &taskNotify, &runningTask]() mutable {
                auto f = [&taskEx, &taskNotify]() mutable {
                    Latch latch(1u);
                    taskNotify.schedule([latchPtr = &latch]() mutable {
                        std::this_thread::sleep_for(1us);
                        latchPtr->downCount();
                    });
                    latch.await(&taskEx);
                };
                std::vector<std::function<void()>> fvec;
                fvec.emplace_back(f);
                fvec.emplace_back(f);
                fvec.emplace_back(f);
                collectAll<Launch::Schedule>(fvec.begin(), fvec.end(), &taskEx);
                std::vector<std::function<void()>> fvec2;
                fvec2.emplace_back(f);
                fvec2.emplace_back(f);
                fvec2.emplace_back(f);
                collectAll<Launch::Current>(fvec2.begin(), fvec2.end(),
                                            &taskEx);

                runningTask.fetch_sub(1u);
            },
            &taskEx);
        std::this_thread::sleep_for(10us);
    }

    while (runningTask) {
    }
}

}  // namespace uthread
}  // namespace async_simple
