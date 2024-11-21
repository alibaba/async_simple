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

TEST(LazyLocalTest, testSimpleLazyLocalNoOwnership) {
    int* i = new int(10);
    async_simple::executors::SimpleExecutor ex(2);
    const auto& type_info = typeid(async_simple::coro::SimpleLazyLocal<int*>);
    auto sub_task = [&]() -> Lazy<> {
        auto localbase = co_await CurrentLazyLocals{};
        EXPECT_NE(localbase, nullptr);
        EXPECT_FALSE(localbase->empty());
        EXPECT_NE(localbase->type(), nullptr);
        EXPECT_EQ(*localbase->type(), type_info);

        int** v = co_await CurrentLazyLocals<int*>{};
        EXPECT_NE(v, nullptr);
        EXPECT_EQ(*v, i);
        EXPECT_EQ(**v, 20);
        **v = 30;
    };

    auto task = [&]() -> Lazy<> {
        auto&& info = co_await CurrentLazyLocalsTypeInfo{};
        EXPECT_NE(info, nullptr);
        EXPECT_TRUE(*info == type_info);
        void* v = co_await CurrentLazyLocals{};
        EXPECT_NE(v, nullptr);
        (*i) = 20;
        co_await sub_task();
        co_return;
    };
    syncAwait(task().via(&ex).setLazyLocal(i));
    EXPECT_EQ(*i, 30);

    std::condition_variable cv;
    std::mutex mut;
    bool done = false;
    task().via(&ex).setLazyLocal(i).start([&](Try<void>) {
        std::unique_lock<std::mutex> lock(mut);
        EXPECT_EQ(*i, 30);
        delete i;
        i = nullptr;
        done = true;
        cv.notify_all();
    });
    std::unique_lock<std::mutex> lock(mut);
    cv.wait(lock, [&]() -> bool { return done == true; });
    EXPECT_EQ(i, nullptr);
}

TEST(LazyLocalTest, testSimpleLazyLocal) {
    async_simple::executors::SimpleExecutor ex(2);
    const auto& type_info =
        typeid(async_simple::coro::SimpleLazyLocal<std::string>);
    auto sub_task = [&]() -> Lazy<> {
        auto localbase = co_await CurrentLazyLocals{};
        EXPECT_NE(localbase, nullptr);
        EXPECT_FALSE(localbase->empty());
        EXPECT_NE(localbase->type(), nullptr);
        EXPECT_EQ(*localbase->type(), type_info);

        std::string* v = co_await CurrentLazyLocals<std::string>{};
        EXPECT_NE(v, nullptr);
        EXPECT_EQ(*v, "20");
        *v = "30";
    };

    auto task = [&]() -> Lazy<> {
        auto&& info = co_await CurrentLazyLocalsTypeInfo{};
        EXPECT_NE(info, nullptr);
        EXPECT_TRUE(*info == type_info);
        auto* u = co_await CurrentLazyLocals<int>{};
        EXPECT_EQ(u, nullptr);
        std::string* v = co_await CurrentLazyLocals<std::string>{};
        EXPECT_NE(v, nullptr);
        EXPECT_EQ(*v, "10");
        (*v) = "20";
        co_await sub_task();
        co_return;
    };
    std::promise<void> p;
    std::future<void> f = p.get_future();
    task()
        .via(&ex)
        .setLazyLocal(std::string{"10"})
        .start([p = std::move(p)](auto&&, LazyLocalBase* local) mutable {
            EXPECT_NE(local, nullptr);
            EXPECT_NE(local->dyn_cast<std::string>(), nullptr);
            EXPECT_EQ(*local->dyn_cast<std::string>(), "30");
            p.set_value();
        });
    f.wait();
}

struct mylocal : public LazyLocalBaseImpl<mylocal> {
    mylocal(std::string sv) : name(std::move(sv)) {}
    std::string& hello() { return name; }
    std::string name;
};

TEST(LazyLocalTest, testMyLazyLocal) {
    const auto& type_info = typeid(mylocal);
    async_simple::executors::SimpleExecutor ex(2);
    auto sub_task = [&]() -> Lazy<> {
        auto localbase = co_await CurrentLazyLocals{};
        EXPECT_NE(localbase, nullptr);
        EXPECT_FALSE(localbase->empty());
        EXPECT_NE(localbase->type(), nullptr);
        EXPECT_EQ(*localbase->type(), type_info);

        auto* v = co_await CurrentLazyLocals<mylocal>{};
        EXPECT_NE(v, nullptr);
        EXPECT_EQ(v->hello(), "Hi");
        v->hello() = "Hey";
    };
    auto task = [&]() -> Lazy<> {
        auto&& info = co_await CurrentLazyLocalsTypeInfo{};
        EXPECT_NE(info, nullptr);
        EXPECT_TRUE(*info == type_info);
        auto* u = co_await CurrentLazyLocals<int>{};
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
        [p = std::move(p)](auto&&, LazyLocalBase* local) mutable {
            EXPECT_NE(local, nullptr);
            EXPECT_EQ(local->dyn_cast<std::string>(), nullptr);
            EXPECT_NE(local->dyn_cast<mylocal>(), nullptr);
            EXPECT_EQ(local->dyn_cast<mylocal>()->hello(), "Hey");
            p.set_value();
        });
    f.wait();
}

}  // namespace async_simple::coro
