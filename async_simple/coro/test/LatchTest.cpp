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

#include "async_simple/coro/Latch.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

namespace async_simple::coro {

class LatchTest : public FUTURE_TESTBASE {
public:
    LatchTest() : _executor(4) {}
    void caseSetUp() override {}
    void caseTearDown() override {}

    executors::SimpleExecutor _executor;
};

TEST_F(LatchTest, testWorkDone) {
    struct job {
        const std::string name;
        std::string product{"not worked"};

    } jobs[] = {{"annika"}, {"buru"}, {"chuck"}};

    Latch work_done{std::size(jobs)};
    Latch start_up{1};

    auto work = [&](job& my_job) -> Lazy<> {
        co_await start_up.wait();
        EXPECT_EQ(my_job.product, my_job.name + " worked");

        my_job.product = my_job.name + " done";
        co_await work_done.count_down();
    };

    std::cout << "Work starting... ";
    for (auto& job : jobs) {
        auto lazy = work(job).via(&_executor);
        lazy.detach();
    }

    auto wait_lazy = [&]() -> Lazy<> {
        std::cout << "done\n";
        for (auto& job : jobs) {
            job.product = job.name + " worked";
        }
        co_await start_up.count_down();

        co_await work_done.wait();

        std::cout << "Workers finished... ";
        for (auto const& job : jobs) {
            EXPECT_EQ(job.product, job.name + " done");
        }
        std::cout << "done\n";
    };

    syncAwait(wait_lazy().via(&_executor));
}

}  // namespace async_simple::coro
