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
#ifndef ASYNC_SIMPLE_CORO_LAZY_HELPER_H
#define ASYNC_SIMPLE_CORO_LAZY_HELPER_H

#include <async_simple/Common.h>
#include <async_simple/experimental/coroutine.h>
#include <exception>

namespace async_simple {
namespace coro {

// Sync await on a coroutine, block until coro finished, coroutine result will
// be returned. Do not syncAwait in the same executor with Lazy, this may lead
// to a deadlock.
template <typename LazyType>
inline auto syncAwait(LazyType&& lazy) ->
    typename std::decay_t<LazyType>::ValueType {
    logicAssert(lazy._coro.operator bool(),
                "Lazy do not have a coroutine_handle");
    auto executor = lazy._coro.promise()._executor;
    if (executor) {
        logicAssert(!executor->currentThreadInExecutor(),
                    "do not sync await in the same executor with Lazy");
    }

    util::Condition cond;
    using ValueType = typename std::decay_t<LazyType>::ValueType;

    Try<ValueType> value;
    std::move(std::forward<LazyType>(lazy))
        .start([&cond, &value](Try<ValueType> result) {
            value = std::move(result);
            cond.release();
        });
    cond.acquire();
    return std::move(value).value();
}

}  // namespace coro
}  // namespace async_simple

#include <async_simple/coro/Collect.h>

#endif
