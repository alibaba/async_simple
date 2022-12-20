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
#ifndef ASYNC_SIMPLE_CORO_SEMAHORE_H
#define ASYNC_SIMPLE_CORO_SEMAHORE_H

#include <chrono>
#include <cstddef>
#include "async_simple/coro/ConditionVariable.h"
#include "async_simple/coro/SpinLock.h"

namespace async_simple::coro {

template <std::ptrdiff_t LeastMaxValue =
              std::numeric_limits<std::uint32_t>::max()>
class CountingSemaphore {
public:
    static_assert(LeastMaxValue >= 0);
    static_assert(LeastMaxValue <= std::numeric_limits<std::uint32_t>::max());

    explicit CountingSemaphore(std::ptrdiff_t desired) : count_(desired) {}
    ~CountingSemaphore() = default;
    CountingSemaphore(const CountingSemaphore&) = delete;
    CountingSemaphore& operator=(const CountingSemaphore&) = delete;

    static constexpr ptrdiff_t max() noexcept { return LeastMaxValue; }

    Lazy<void> acquire() noexcept;
    Lazy<void> release(std::ptrdiff_t update = 1) noexcept;

    Lazy<bool> try_acquire() noexcept;

    // TODO:
    template <class Rep, class Period>
    bool try_acquire_for(std::chrono::duration<Rep, Period> expires_in);
    template <class Clock, class Duration>
    bool try_acquire_until(std::chrono::time_point<Clock, Duration> expires_at);

private:
    using MutexType = SpinLock;

    MutexType lock_;
    ConditionVariable<MutexType> cv_;
    std::ptrdiff_t count_;
};

using BinarySemaphore = CountingSemaphore<1>;

template <std::ptrdiff_t LeastMaxValue>
Lazy<void> CountingSemaphore<LeastMaxValue>::acquire() noexcept {
    auto lock = co_await lock_.coScopedLock();
    co_await cv_.wait(lock_, [this] { return count_ > 0; });
    --count_;
}

template <std::ptrdiff_t LeastMaxValue>
Lazy<void> CountingSemaphore<LeastMaxValue>::release(
    std::ptrdiff_t update) noexcept {
    // TODO: check update <= LeastMaxValue && update > 0
    auto lock = co_await lock_.coScopedLock();
    count_ += update;
    if (update > 1) {
        cv_.notifyAll();
    } else {
        cv_.notifyOne();
    }
}

template <std::ptrdiff_t LeastMaxValue>
Lazy<bool> CountingSemaphore<LeastMaxValue>::try_acquire() noexcept {
    auto lock = co_await lock_.coScopedLock();
    if (count_) {
        --count_;
        co_return true;
    }
    co_return false;
}

}  // namespace async_simple::coro

#endif  // ASYNC_SIMPLE_CORO_SEMAHORE_H
