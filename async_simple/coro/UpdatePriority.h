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

#ifndef ASYNC_SIMPLE_CORO_UPDATE_PRIORITY_H
#define ASYNC_SIMPLE_CORO_UPDATE_PRIORITY_H

#include "async_simple/Executor.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/experimental/coroutine.h"

#include <cassert>
#include <type_traits>

namespace async_simple {
namespace coro {

enum class UpdatePriorityOption : uint8_t {
    IMMEDIATELY,
    DELAYED,
};

namespace detail {

class UpdatePriorityAwaiter {
public:
    explicit UpdatePriorityAwaiter(int8_t new_priority, Executor* ex,
                                   UpdatePriorityOption option) noexcept
        : _new_priority(new_priority), _ex(ex), _option(option) {
        assert(_ex != nullptr);
    }

    bool await_ready() const noexcept { return false; }

    template <typename PromiseType>
    bool await_suspend(std::coroutine_handle<PromiseType> h) {
        static_assert(std::is_base_of<LazyPromiseBase, PromiseType>::value,
                      "updatePriority is only allowed to be called by Lazy");
        int8_t old_priority = h.promise()._priority;
        if (_new_priority == old_priority) {
            return false;
        }
        h.promise()._priority = _new_priority;
        if (_option == UpdatePriorityOption::DELAYED) {
            return false;
        }
        assert(_option == UpdatePriorityOption::IMMEDIATELY);
        bool succ = _ex->scheduleWithPriority(std::move(h), _new_priority);
        if (succ == false)
            AS_UNLIKELY {
                throw std::runtime_error(
                    "updatePriority dispatch to executor failed");
            }
        return true;
    }

    void await_resume() noexcept {}

private:
    int8_t _new_priority;
    Executor* _ex;
    UpdatePriorityOption _option;
};

class UpdatePriorityAwaitable {
public:
    explicit UpdatePriorityAwaitable(int8_t new_priority,
                                     UpdatePriorityOption option)
        : _new_priority(new_priority), _option(option) {}

    auto coAwait(Executor* ex) {
        return UpdatePriorityAwaiter(_new_priority, ex, _option);
    }

private:
    int8_t _new_priority;
    UpdatePriorityOption _option;
};

}  // namespace detail

inline detail::UpdatePriorityAwaitable updatePriority(
    int8_t new_priority, UpdatePriorityOption option) {
    return detail::UpdatePriorityAwaitable(new_priority, option);
}

}  // namespace coro
}  // namespace async_simple

#endif
