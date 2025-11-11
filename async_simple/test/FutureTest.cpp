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
#include <exception>
#include <functional>
#include <mutex>

#include <vector>

#ifdef USE_MODULES

#include <atomic>
#include "async_simple/test/unittest.h"
import async_simple;
#else
#include "async_simple/Collect.h"
#include "async_simple/Future.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"
#endif

using namespace std;
using namespace async_simple::executors;

namespace async_simple {

class FutureTest : public FUTURE_TESTBASE {
public:
    void caseSetUp() override {}
    void caseTearDown() override {}

    template <typename T>
    void doTestType(bool readyFuture);
};

namespace {

enum DummyState : int {
    CONSTRUCTED = 1,
    DESTRUCTED = 2,
};

struct Dummy {
    Dummy() : state(nullptr), value(0) {}
    Dummy(int x) : state(nullptr), value(x) {}
    Dummy(int* state_) : state(state_), value(0) {
        if (state) {
            (*state) |= CONSTRUCTED;
        }
    }
    Dummy(Dummy&& other) : state(other.state), value(other.value) {
        other.state = nullptr;
    }
    Dummy& operator=(Dummy&& other) {
        if (this != &other) {
            std::swap(other.state, state);
            std::swap(other.value, value);
        }
        return *this;
    }

    ~Dummy() {
        if (state) {
            (*state) |= DESTRUCTED;
            state = nullptr;
        }
    }

    Dummy(const Dummy&) = delete;
    Dummy& operator=(const Dummy&) = delete;

    int* state = nullptr;
    int value = 0;

    Dummy& operator+(const int& rhs) & {
        value += rhs;
        return *this;
    }
    Dummy&& operator+(const int& rhs) && {
        value += rhs;
        return std::move(*this);
    }
    Dummy& operator+(const Dummy& rhs) & {
        value += rhs.value;
        return *this;
    }
    Dummy&& operator+(const Dummy& rhs) && {
        value += rhs.value;
        return std::move(*this);
    }

    bool operator==(const Dummy& other) const { return value == other.value; }
};
}  // namespace

TEST_F(FutureTest, testSimpleProcess) {
    SimpleExecutor executor(5);

    Promise<int> p;
    auto future = p.getFuture();
    EXPECT_TRUE(p.valid());

    int output = 0;
    auto f = std::move(future).via(&executor).thenTry([&output](Try<int>&& t) {
        output = t.value();
        return 123;
    });

    p.setValue(456);

    f.wait();
    const Try<int>& t = f.result();
    EXPECT_TRUE(t.available());
    EXPECT_FALSE(t.hasError());

    EXPECT_EQ(123, std::move(f).get());
    EXPECT_EQ(456, output);
}

TEST_F(FutureTest, testGetSet) {
    Promise<int> p;
    auto f = p.getFuture();
    ASSERT_THROW(p.getFuture(), std::logic_error);
    p.setValue(456);
    f.wait();
    EXPECT_EQ(456, f.value());
}

TEST_F(FutureTest, testThenValue) {
    SimpleExecutor executor(5);

    Promise<int> p;
    auto future = p.getFuture();
    EXPECT_TRUE(p.valid());

    int output = 0;
    auto f =
        std::move(future).via(&executor).thenValue([&output](const int64_t& t) {
            output = t;
            return 123;
        });

    p.setValue(456);

    f.wait();
    const Try<int>& t = f.result();
    EXPECT_TRUE(t.available());
    EXPECT_FALSE(t.hasError());

    EXPECT_EQ(123, std::move(f).get());
    EXPECT_EQ(456, output);
}

TEST_F(FutureTest, testChainedFuture) {
    SimpleExecutor executor(5);
    Promise<int> p;
    int output0 = 0;
    int output1 = 0;
    int output2 = 0;
    std::vector<int> order;
    std::mutex mtx;
    auto record = [&order, &mtx](int x) {
        std::lock_guard<std::mutex> l(mtx);
        order.push_back(x);
    };
    auto future = p.getFuture().via(&executor);
    auto f = std::move(future)
                 .thenTry([&output0, record](Try<int>&& t) {
                     record(0);
                     output0 = t.value();
                     return t.value() + 100;
                 })
                 .thenTry([&output1, &executor, record](Try<int>&& t) {
                     record(1);
                     output1 = t.value();
                     Promise<int> p;
                     auto f = p.getFuture().via(&executor);
                     p.setValue(t.value() + 10);
                     return f;
                 })
                 .thenValue([&output2, record](int x) {
                     record(2);
                     output2 = x;
                     return std::to_string(x);
                 })
                 .thenValue([](string&& s) { return 1111.0; });
    p.setValue(1000);
    f.wait();
    EXPECT_EQ(3u, order.size());
    int last = -1;
    for (auto a : order) {
        EXPECT_LT(last, a);
        last = a;
    }
    EXPECT_EQ(1000, output0);
    EXPECT_EQ(1100, output1);
    EXPECT_EQ(1110, output2);
    EXPECT_EQ(1111.0, std::move(f).get());
}

template <typename T>
void FutureTest::doTestType(bool readyFuture) {
    SimpleExecutor executor(5);
    Promise<T> p;
    std::vector<int> order;
    std::mutex mtx;
    auto record = [&order, &mtx](int x) {
        std::lock_guard<std::mutex> l(mtx);
        order.push_back(x);
    };
    auto future = p.getFuture().via(&executor);
    if (readyFuture) {
        future = makeReadyFuture(T(1000));
    }
    auto f = std::move(future)
                 .thenTry([record](Try<T>&& t) mutable {
                     record(0);
                     return std::move(t).value() + 100;
                 })
                 .thenTry([&executor, record](Try<T> t) mutable {
                     record(1);
                     Promise<T> p;
                     auto f = p.getFuture().via(&executor);
                     p.setValue(std::move(t).value() + 10);
                     return f;
                 })
                 .thenValue([record](T&& x) mutable {
                     record(2);
                     return std::move(x) + 1;
                 });
    p.setValue(T(1000));
    f.wait();
    EXPECT_EQ(3u, order.size());
    int last = -1;
    for (auto a : order) {
        EXPECT_LT(last, a);
        last = a;
    }
    EXPECT_EQ(T(1111), std::move(f).get());
}
TEST_F(FutureTest, testClass) {
    doTestType<int>(true);
    doTestType<Dummy>(true);
    doTestType<int>(false);
    doTestType<Dummy>(false);
}

TEST_F(FutureTest, testException) {
    SimpleExecutor executor(5);

    Promise<int> p;
    auto future = p.getFuture().via(&executor);
    EXPECT_TRUE(p.valid());

    auto f = std::move(future)
                 .thenTry([](Try<int> x) { return x.value() + 100; })
                 .thenValue([](int x) { return x + 10; })
                 .thenTry([](Try<int> x) {
                     try {
                         return x.value() + 1.0;
                     } catch (...) {
                         return -1.0;
                     }
                 });

    try {
        throw std::runtime_error("FAILED");
    } catch (...) {
        p.setException(std::current_exception());
    }

    f.wait();
    const Try<double>& t = f.result();
    EXPECT_TRUE(t.available());
    EXPECT_FALSE(t.hasError());
    EXPECT_EQ(-1.0, std::move(f).get());
}

TEST_F(FutureTest, testVoid) {
    SimpleExecutor executor(5);

    Promise<int> p;
    auto future = p.getFuture().via(&executor);
    EXPECT_TRUE(p.valid());
    int output = 0;
    auto f = std::move(future)
                 .thenTry([&output](Try<int> x) { output = x.value(); })
                 .thenTry([](auto&&) { return 200; });

    p.setValue(100);
    f.wait();
    EXPECT_EQ(200, std::move(f).get());
    EXPECT_EQ(100, output);
}

// FIXME: It looks problematic to use `#include <thread>` with
// `import async_simple;` now. There are some problem in the compiler side.
#ifndef USE_MODULES
TEST_F(FutureTest, testWait) {
    SimpleExecutor executor(5);
    int output;
    Promise<int> p;
    auto future = p.getFuture().via(&executor);
    EXPECT_TRUE(p.valid());
    std::atomic<bool> beginCallback(false);
    std::atomic<int> doneCallback(0);
    auto f = std::move(future).thenTry(
        [&output, &beginCallback, &doneCallback](Try<int> x) {
            int tmp = doneCallback.load(std::memory_order_acquire);
            while (!doneCallback.compare_exchange_weak(tmp, 1)) {
                tmp = doneCallback.load(std::memory_order_acquire);
            }
            while (!beginCallback.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            output = x.value();
            return output + 5;
        });
    // use doneCallback to know which step future finish.
    // use beginCallback to notify future start callback function.
    auto t = std::thread([&p, &beginCallback, &doneCallback]() {
        std::this_thread::sleep_for(100000us);
        // sleep a little time and make sure the future callback do not start.
        EXPECT_EQ(0, doneCallback.load(std::memory_order_acquire));
        p.setValue(100);
        for (size_t i = 5;
             i > 0 && doneCallback.load(std::memory_order_acquire) != 1; i--) {
            std::this_thread::sleep_for(1000us);
        }
        // the future callback has started but can not finish.
        EXPECT_EQ(1, doneCallback.load(std::memory_order_acquire));
        // notify future finish callback
        beginCallback.store(true, std::memory_order_release);
        for (size_t i = 500;
             i > 0 && doneCallback.load(std::memory_order_acquire) != 2; i--) {
            std::this_thread::sleep_for(10000us);
        }
        // make sure future callback has finished
        EXPECT_EQ(2, doneCallback.load(std::memory_order_acquire));
    });
    f.wait();
    doneCallback.store(2, std::memory_order_release);
    EXPECT_EQ(105, std::move(f).get());
    EXPECT_EQ(100, output);
    t.join();
}

TEST_F(FutureTest, testWaitCallback) {
    SimpleExecutor executor(2), executor2(1);
    Promise<int> p;
    auto future = p.getFuture().via(&executor);
    EXPECT_TRUE(p.valid());
    Promise<bool> p2;
    auto f =
        std::move(future)
            .thenTry([&p2, &executor2](Try<int> res) {
                auto f = p2.getFuture()
                             .via(&executor2)
                             .thenValue([x = std::move(res).value()](bool y) {
                                 std::this_thread::sleep_for(10000us);
                                 return x;
                             });
                return f;
            })
            .thenValue([](int x) {
                std::this_thread::sleep_for(20000us);
                return std::make_pair(x + 1, x);
            })
            .thenValue([&executor2](std::pair<int, int>&& res) {
                // return res.first * res.second;
                Promise<bool> p3;
                Future<int> f = p3.getFuture()
                                    .via(&executor2)
                                    .thenValue([r = std::move(res)](bool y) {
                                        std::this_thread::sleep_for(30000us);
                                        return r.first * r.second;
                                    });
                p3.setValue(true);
                // return std::move(f).get();
                return f;
            });
    p.setValue(2);
    p2.setValue(true);
    f.wait();
    ASSERT_EQ(6, std::move(f).get());
}
#endif

TEST_F(FutureTest, testCollectAll) {
    SimpleExecutor executor(15);

    size_t n = 10;
    vector<Promise<Dummy>> promise(n);
    vector<Future<Dummy>> futures;
    for (size_t i = 0; i < n; ++i) {
        futures.push_back(promise[i].getFuture().via(&executor));
    }
    vector<int> expected;
    for (size_t i = 0; i < n; ++i) {
        expected.push_back(i);
    }
    auto f = collectAll(futures.begin(), futures.end())
                 .thenValue([&expected](vector<Try<Dummy>>&& vec) {
                     EXPECT_EQ(expected.size(), vec.size());
                     for (size_t i = 0; i < vec.size(); ++i) {
                         EXPECT_EQ(expected[i], vec[i].value().value);
                     }
                     expected.clear();
                 });

    for (size_t i = 0; i < n; ++i) {
        promise[i].setValue(Dummy(i));
    }

    f.wait();

    EXPECT_TRUE(expected.empty());
}

TEST_F(FutureTest, testCollectAllVoid) {
    SimpleExecutor executor(15);

    size_t n = 10;
    vector<Promise<void>> promise(n);
    vector<Future<void>> futures;
    for (size_t i = 0; i < n; ++i) {
        futures.push_back(promise[i].getFuture().via(&executor));
    }
    vector<int> expected;
    for (size_t i = 0; i < n; ++i) {
        expected.push_back(i);
    }
    auto f = collectAll(futures.begin(), futures.end())
                 .thenValue([&expected](vector<Try<void>>&& vec) {
                     EXPECT_EQ(expected.size(), vec.size());
                     expected.clear();
                 });

    for (size_t i = 0; i < n; ++i) {
        promise[i].setValue();
    }

    f.wait();

    EXPECT_TRUE(expected.empty());
}

TEST_F(FutureTest, testCollectReadyFutures) {
    SimpleExecutor executor(15);

    size_t n = 10;
    vector<Future<Dummy>> futures;
    for (size_t i = 0; i < n; ++i) {
        futures.push_back(makeReadyFuture<Dummy>(i));
    }
    bool executed = false;
    auto f = collectAll(futures.begin(), futures.end())
                 .thenValue([&executed, n](vector<Try<Dummy>>&& vec) {
                     EXPECT_EQ(n, vec.size());
                     for (size_t i = 0; i < vec.size(); ++i) {
                         EXPECT_EQ(
                             i, static_cast<decltype(i)>(vec[i].value().value));
                     }
                     executed = true;
                 });
    EXPECT_TRUE(f.TEST_hasLocalState());
    f.wait();
    EXPECT_TRUE(executed);
}

TEST_F(FutureTest, testPromiseBroken) {
    Promise<Dummy> p;
    auto f = p.getFuture();
    {
        // destruct p
        auto innerP = std::move(p);
        (void)innerP;
    }
    f.wait();
    auto& r = f.result();
    EXPECT_TRUE(r.available());
    EXPECT_TRUE(r.hasError());
}

#ifndef USE_MODULES
TEST_F(FutureTest, testViaAfterWait) {
    Promise<int> promise;
    auto future = promise.getFuture();

    auto t = std::thread([p = std::move(promise)]() mutable {
        std::this_thread::sleep_for(1s);
        p.setValue(100);
    });

    future.wait();
    std::move(future).via(nullptr).thenValue(
        [](int v) mutable { ASSERT_EQ(100, v); });
    t.join();
}
#endif

TEST_F(FutureTest, testReadyFuture) {
    auto future = makeReadyFuture(3);
    future.wait();
    std::move(future).via(nullptr).thenValue(
        [](int v) mutable { ASSERT_EQ(3, v); });
    auto future2 = makeReadyFuture();
    EXPECT_TRUE(future2.hasResult());
    EXPECT_TRUE(future2.valid());
}

TEST_F(FutureTest, testPromiseCopy) {
    auto promise1 = std::make_unique<Promise<int>>();
    auto promise2 = std::make_unique<Promise<int>>();
    promise2->setValue(0);
    auto future = promise1->getFuture();
    *promise1 = *promise2;
    promise1.reset();
    ASSERT_THROW(future.value(), std::runtime_error);
    auto promise3 = *promise2;
    promise2.reset();
    EXPECT_EQ(0, promise3.getFuture().value());
}

TEST_F(FutureTest, testVoidFuture) {
    Promise<void> p;
    auto f = p.getFuture();
    int i = 0;
    EXPECT_FALSE(f.hasResult());
    f.setContinuation([&i](Try<void>) { EXPECT_EQ(i, 5); });
    i = 5;
    p.setValue();
    EXPECT_TRUE(f.hasResult());

    Promise<void> p2;
    auto f2 = p2.getFuture()
                  .thenTry([](Try<void>) {

                  })
                  .thenTry([](Try<void> t) { return t.value(); })
                  .thenValue([]() { return 10; })
                  .thenValue([](int i) { EXPECT_EQ(i, 10); });
    p2.setValue();
    f2.wait();

    Promise<void> p3;
    auto f3 = p3.getFuture();
    ASSERT_THROW(f3.value(), std::logic_error);
    auto f4 = std::move(f3)
                  .thenTry([](Try<void> t) { return t.value(); })
                  .thenTry([](Try<void> t) {
                      try {
                          t.value();
                          return 0;
                      } catch (...) {
                          return 1;
                      }
                  });

    try {
        throw std::runtime_error("test exception");
    } catch (...) {
        p3.setException(std::current_exception());
    }

    f4.wait();
    const Try<int>& t = f4.result();
    EXPECT_TRUE(t.available());
    EXPECT_FALSE(t.hasError());
    EXPECT_EQ(1, std::move(f4).get());
}

TEST_F(FutureTest, testThen) {
    Promise<int> p1;
    auto f1 = p1.getFuture()
                  .then([](int i) { return i * 2; })
                  .then([](Try<int> t) { return t.value() * 2; })
                  .then([](int i) { return i; });
    p1.setValue(1);
    EXPECT_EQ(f1.value(), 4);
}
}  // namespace async_simple
