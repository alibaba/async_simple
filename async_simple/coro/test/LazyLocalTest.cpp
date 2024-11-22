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
#include "async_simple/coro/LazyLocalBase.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

#include <condition_variable>
#include <future>
#include <mutex>
#include <string>

namespace async_simple::coro {

struct mylocal : public LazyLocalBase {
    mylocal(std::string sv) : LazyLocalBase(&tag), name(std::move(sv)) {}
    std::string& hello() { return name; }
    std::string name;
    static bool classof(LazyLocalBase* base) {
        return base->getTypeTag() == &tag;
    }
    inline static char tag;
};

struct mylocal2 : public LazyLocalBase {
    mylocal2(std::string sv) : LazyLocalBase(&tag), name(std::move(sv)) {}
    std::string& hello() { return name; }
    std::string name;
    static bool classof(LazyLocalBase* base) {
        return base->getTypeTag() == &tag;
    }
    inline static char tag;
};

TEST(LazyLocalTest, testMyLazyLocal) {
    async_simple::executors::SimpleExecutor ex(2);
    auto sub_task = [&]() -> Lazy<> {
        auto localbase = co_await CurrentLazyLocals{};
        EXPECT_NE(localbase, nullptr);
        EXPECT_NE(localbase->getTypeTag(), nullptr);

        auto* v = co_await CurrentLazyLocals<mylocal>{};
        EXPECT_NE(v, nullptr);
        EXPECT_EQ(v->hello(), "Hi");
        v->hello() = "Hey";
    };
    auto task = [&]() -> Lazy<> {
        auto* u = co_await CurrentLazyLocals<mylocal2>{};
        EXPECT_EQ(u, nullptr);
        auto* v = co_await CurrentLazyLocals<mylocal>{};
        EXPECT_NE(v, nullptr);
        EXPECT_EQ(v->hello(), "Hello");
        v->hello() = "Hi";
        co_await sub_task();
        co_return;
    };
    std::promise<void> p;
    std::future<void> f = p.get_future();
    task().setLazyLocal<mylocal>("Hello").start(
        [p = std::move(p)](auto&&) mutable { p.set_value(); });
}

TEST(LazyLocalTest, testSetLazyLocalTwice) {
    auto task = [&]() -> Lazy<> {
        auto* v = co_await CurrentLazyLocals<mylocal>{};
        EXPECT_NE(v, nullptr);
        EXPECT_EQ(v->hello(), "1");
        co_return;
    };
    bool has_error = false;
    try {
        syncAwait(task().setLazyLocal<mylocal>("2").setLazyLocal<mylocal>("1"));
    } catch (...) {
        has_error = true;
    }
    EXPECT_TRUE(has_error);
}

TEST(LazyLocalTest, testSetLazyLocalNested) {
    auto sub_task = [&]() -> Lazy<> { co_return; };
    auto task = [&]() -> Lazy<void> {
        auto* v = co_await CurrentLazyLocals<mylocal>{};
        EXPECT_NE(v, nullptr);
        EXPECT_EQ(v->hello(), "1");

        bool has_error = false;
        try {
            co_await sub_task().setLazyLocal<mylocal>("2");
        } catch (...) {
            has_error = true;
        }
        EXPECT_TRUE(has_error);
        co_return;
    };

    syncAwait(task().setLazyLocal<mylocal>("1"));
}

TEST(LazyLocalTest, testSetLazyLocalButDoNothing) {
    auto task = [&]() -> Lazy<void> { co_return; };
    auto task2 = task().setLazyLocal<mylocal>("1");
}

}  // namespace async_simple::coro
