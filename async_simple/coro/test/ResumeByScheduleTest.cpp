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

#include "async_simple/Promise.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/ResumeBySchedule.h"
#include "async_simple/executors/SimpleExecutor.h"

#include "gtest/gtest.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

using namespace async_simple::executors;

namespace async_simple {
namespace coro {

class CallBackSystem {
public:
    using Func = std::function<void()>;

    CallBackSystem() : stop_(false) {
        backend_ = std::thread([this]() {
            while (true) {
                std::vector<Func> tasks;
                std::unique_lock<std::mutex> guard(this->mut_);
                cv_.wait(guard, [&]() {
                    return this->tasks_.empty() == false || this->stop_ == true;
                });
                if (this->tasks_.empty() == true && this->stop_ == true) {
                    return;
                }
                tasks.swap(this->tasks_);
                guard.unlock();
                for (auto&& each : tasks) {
                    each();
                }
            }
        });
    }

    void Call(Func func) {
        std::unique_lock<std::mutex> guard(this->mut_);
        tasks_.push_back(std::move(func));
        cv_.notify_one();
    }

    void Stop() {
        std::unique_lock<std::mutex> guard(this->mut_);
        if (stop_ == true) {
            return;
        }
        stop_ = true;
        cv_.notify_one();
        guard.unlock();
        backend_.join();
    }

private:
    std::thread backend_;
    std::mutex mut_;
    std::condition_variable cv_;
    bool stop_;
    std::vector<Func> tasks_;
};

class MockExecutorForResumeBySchedule : public SimpleExecutor {
public:
    using Base = SimpleExecutor;

    explicit MockExecutorForResumeBySchedule(size_t thread_num)
        : SimpleExecutor(thread_num), schedule_count_(0), checkin_count_(0) {}

    bool schedule(Func func) override {
        ++schedule_count_;
        return Base::schedule(std::move(func));
    }

    bool checkin(Func func, Base::Context ctx, ScheduleOptions opts) override {
        ++checkin_count_;
        return Base::checkin(std::move(func), ctx, opts);
    }

    size_t schedule_count_;
    size_t checkin_count_;
};

TEST(ResumeBySchedule, basic) {
    MockExecutorForResumeBySchedule ex(2);
    CallBackSystem cbs;

    auto task = [&cbs]() -> Lazy<void> {
        Promise<int> pr;
        auto fu = pr.getFuture();
        cbs.Call([pr = std::move(pr)]() mutable { pr.setValue(1); });
        int v = co_await ResumeBySchedule(std::move(fu));
        EXPECT_EQ(v, 1);
        co_return;
    };

    std::mutex mut;
    std::condition_variable cv;
    size_t done_count = 0;

    for (size_t i = 0; i < 100; ++i) {
        task().via(&ex).start([&](auto&&) {
            std::unique_lock guard(mut);
            done_count += 1;
            cv.notify_one();
        });
    }

    std::unique_lock guard(mut);
    cv.wait(guard, [&]() -> bool { return done_count == 100; });
    cbs.Stop();
    EXPECT_EQ(ex.checkin_count_, 0);
    EXPECT_LE(ex.schedule_count_, 200);
}

}  // namespace coro
}  // namespace async_simple
