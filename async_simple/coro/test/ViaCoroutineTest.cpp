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
#include <type_traits>
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

#include <iostream>
#include <thread>

using namespace std;

namespace async_simple {

namespace coro {

class ViaCoroutineTest : public FUTURE_TESTBASE {
public:
    void caseSetUp() override {}
    void caseTearDown() override {}
};

std::atomic<int> check{0};
class SimpleExecutorTest : public executors::SimpleExecutor {
public:
    SimpleExecutorTest(size_t tn) : SimpleExecutor(tn) {}
    Context checkout() override {
        check++;
        return SimpleExecutor::checkout();
    }
    bool checkin(Func func, Context ctx, ScheduleOptions opts) override {
        // -1 is invalid ctx for SimpleExecutor
        if (ctx == (void*)-1) {
            return false;
        }
        check--;
        return SimpleExecutor::checkin(func, ctx, opts);
    }
};

class Awaiter {
public:
    bool await_ready() noexcept { return false; }
    bool await_suspend(std::coroutine_handle<> continuation) noexcept {
        return false;
    }
    void await_resume() noexcept {}
};

TEST_F(ViaCoroutineTest, SimplecheckoutEQcheckin) {
    SimpleExecutorTest e1(10);
    auto Task = [&]() -> Lazy<> { co_await Awaiter(); };
    syncAwait(Task().via(&e1));
    EXPECT_EQ(check.load(), 0);
}

}  // namespace coro
}  // namespace async_simple
