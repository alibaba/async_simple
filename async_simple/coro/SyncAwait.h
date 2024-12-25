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
#ifndef ASYNC_SIMPLE_CORO_SYNC_AWAIT_H
#define ASYNC_SIMPLE_CORO_SYNC_AWAIT_H

#ifndef ASYNC_SIMPLE_USE_MODULES
#include <condition_variable>
#include <mutex>
#include <type_traits>
#include <utility>
#include "async_simple/Common.h"
#include "async_simple/Executor.h"
#include "async_simple/Try.h"
#include "async_simple/executors/LocalExecutor.h"
#include "async_simple/util/Condition.h"

#endif  // ASYNC_SIMPLE_USE_MODULES

namespace async_simple {
namespace coro {

// Sync await on a coroutine, block until coro finished, coroutine result will
// be returned. Do not syncAwait in the same executor with Lazy, this may lead
// to a deadlock.
template <typename LazyType>
requires std::remove_cvref_t<LazyType>::isReschedule inline auto syncAwait(
    LazyType &&lazy) {
    auto executor = lazy.getExecutor();
    if (executor)
        logicAssert(!executor->currentThreadInExecutor(),
                    "do not sync await in the same executor with Lazy");

    util::Condition cond;
    using ValueType = typename std::remove_cvref_t<LazyType>::ValueType;

    Try<ValueType> value;
    std::move(std::forward<LazyType>(lazy))
        .start([&cond, &value](Try<ValueType> result) {
            value = std::move(result);
            cond.release();
        });
    cond.acquire();
    return std::move(value).value();
}

/*
 * note:
 * 有问题，因为用户可能自定义awaiter，将resume交给其他执行流执行，因此不能确定loop退出后还有无协程调度，这种情况下会导致死锁。
 * 并且调度器需要保证线程安全，因为可能resume的时候将会在其他线程schedule。
 * 解决办法：1.
 * cv+mutex+quit_flag，这种情况对性能有损伤（不过现代架构下锁争抢是性能衰退的主要原因，无冲突的加解锁可以接受）。
 *          2.
 * 由用户保证awaiter不会在其他线程/执行流resume和schedule，这种情况没法显式约束。
 */
/*
 * update:
 * 目前是cv+mutex_quit_flag的实现。
 */
template <typename LazyType>
requires(!std::remove_cvref_t<LazyType>::isReschedule) inline auto syncAwait(
    LazyType &&lazy) {
    std::condition_variable cv;
    std::mutex mut;
    bool quit_flag{false};
    executors::LocalExecutor local_ex(cv, mut, quit_flag);
    using ValueType = typename std::remove_cvref_t<LazyType>::ValueType;
    Try<ValueType> value;
    std::move(std::forward<LazyType>(lazy))
        .via(&local_ex)
        .start([&cv, &mut, &quit_flag, &value](Try<ValueType> result) {
            value = std::move(result);
            std::unique_lock<std::mutex> lock(mut);
            quit_flag = true;
            cv.notify_all();
        });
    local_ex.Loop();
    return std::move(value).value();
}

// A simple wrapper to ease the use.
template <typename LazyType>
inline auto syncAwait(LazyType &&lazy, Executor *ex) {
    return syncAwait(std::move(std::forward<LazyType>(lazy)).via(ex));
}

}  // namespace coro
}  // namespace async_simple

#endif
