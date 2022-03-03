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
#ifndef ASYNC_SIMPLE_EXECUTOR_H
#define ASYNC_SIMPLE_EXECUTOR_H

#include <async_simple/IOExecutor.h>
#include <async_simple/experimental/coroutine.h>
#include <async_simple/util/Condition.h>
#include <chrono>
#include <functional>
#include <string>
#include <thread>

namespace async_simple {
// Stat information for an executor.
// It contains the number of pending task
// for the executor now.
struct ExecutorStat {
    size_t pendingTaskCount = 0;
    ExecutorStat() = default;
};
// Options for a schedule.
// The option contains:
// - bool prompt. Whether or not this schedule
//   should be prompted.
struct ScheduleOptions {
    bool prompt = true;
    ScheduleOptions() = default;
};

// Awaitable to get the current executor.
// For example:
// ```
//  auto current_executor =
//      co_await CurrentExecutor{};
// ```
struct CurrentExecutor {};

// Executor is a scheduler for functions.
//
// Executor is a key component for scheduling coroutines.
// Considering that there should be already an exeuctor
// in most production-level programs, Executor is designed
// to be able to fit the scheduling strategy in existing programs.
//
// User should derive from Executor and implement their scheduling
// strategy.
class Executor {
public:
    // Context is an indentification for the context where an executor
    // should run. See checkin/checkout for details.
    using Context = void *;
    static constexpr Context NULLCTX = nullptr;

    // A time duration in microseconds.
    using Duration = std::chrono::duration<int64_t, std::micro>;

    // The schedulable function. Func should accept no argument and
    // return void.
    using Func = std::function<void()>;
    class Awaitable;
    class Awaiter;
    class TimeAwaitable;
    class TimeAwaiter;

    Executor(std::string name = "default") : _name(std::move(name)) {}
    virtual ~Executor() {}

    Executor(const Executor &) = delete;
    Executor &operator=(const Executor &) = delete;

    // Schedule a function.
    // `schedule` would return false if schedule failed, which means function
    // func will not be executed. In case schedule return true, the executor
    // should guarantee that the func would be executed.
    virtual bool schedule(Func func) = 0;
    // Return true if caller runs in the executor.
    virtual bool currentThreadInExecutor() const {
        throw std::logic_error("Not implemented");
    }
    virtual ExecutorStat stat() const {
        throw std::logic_error("Not implemented");
    }

    // checkout() return current "Context", which defined by executor
    // implementation, then checkin(func, "Context") should schdule func to the
    // same "Context" as before.
    virtual size_t currentContextId() const { return 0; };
    virtual Context checkout() { return NULLCTX; }
    virtual bool checkin(Func func, Context ctx, ScheduleOptions opts) {
        return schedule(std::move(func));
    }
    virtual bool checkin(Func func, Context ctx) {
        static ScheduleOptions opts;
        return checkin(std::move(func), ctx, opts);
    }

    const std::string &name() const { return _name; }

    // Schedule inside a coroutine.
    // Use
    //  co_await executor.schedule()
    // to schedule current execution
    //
    // Use
    //  co_await executor.after(sometime)
    // to schedule current execution after some time.
    Awaitable schedule();
    TimeAwaitable after(Duration dur);

    // IOExecutor accepts IO read/write requests.
    // Return nullptr if the exeuctor doesn't offer an IOExecutor.
    virtual IOExecutor *getIOExecutor() {
        throw std::logic_error("Not implemented");
    }

    // This method will block current thread until func complete.
    // Return false if it fails to schedule func. Return true
    // otherwise.
    bool syncSchedule(Func func) {
        util::Condition cond;
        auto ret = schedule([f = std::move(func), &cond]() {
            f();
            cond.release();
        });
        if (!ret) {
            return false;
        }
        cond.acquire();
        return true;
    }

private:
    virtual void schedule(Func func, Duration dur) {
        std::thread([this, func = std::move(func), dur]() {
            std::this_thread::sleep_for(dur);
            schedule(std::move(func));
        }).detach();
    }

    std::string _name;
};

// Awaiter to implement Executor::schedule.
class Executor::Awaiter {
public:
    Awaiter(Executor *ex) : _ex(ex) {}

public:
    bool await_ready() const noexcept {
        if (_ex->currentThreadInExecutor()) {
            return true;
        }
        return false;
    }

    template <typename PromiseType>
    void await_suspend(STD_CORO::coroutine_handle<PromiseType> continuation) {
        std::function<void()> func = [c = continuation]() mutable {
            c.resume();
        };
        _ex->schedule(func);
    }
    void await_resume() const noexcept {}

private:
    Executor *_ex;
};

// Awaitable to implement Executor::schedule.
class Executor::Awaitable {
public:
    Awaitable(Executor *ex) : _ex(ex) {}

    auto coAwait(Executor *) { return Executor::Awaiter(_ex); }

private:
    Executor *_ex;
};

Executor::Awaitable inline Executor::schedule() {
    return Executor::Awaitable(this);
};

// Awaiter to implement Executor::after.
class Executor::TimeAwaiter {
public:
    TimeAwaiter(Executor *ex, Executor::Duration dur) : _ex(ex), _dur(dur) {}

public:
    bool await_ready() const noexcept { return false; }

    template <typename PromiseType>
    void await_suspend(STD_CORO::coroutine_handle<PromiseType> continuation) {
        std::function<void()> func = [c = continuation]() mutable {
            c.resume();
        };
        _ex->schedule(func, _dur);
    }
    void await_resume() const noexcept {}

private:
    Executor *_ex;
    Executor::Duration _dur;
};

// Awaitable to implement Executor::after.
class Executor::TimeAwaitable {
public:
    TimeAwaitable(Executor *ex, Executor::Duration dur) : _ex(ex), _dur(dur) {}

    auto coAwait(Executor *) { return Executor::TimeAwaiter(_ex, _dur); }

private:
    Executor *_ex;
    Executor::Duration _dur;
};

Executor::TimeAwaitable inline Executor::after(Executor::Duration dur) {
    return Executor::TimeAwaitable(this, dur);
};

}  // namespace async_simple

#endif
