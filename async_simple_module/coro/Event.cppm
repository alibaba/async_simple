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
module;

module async_simple:coro.Event;

import experimental.coroutine;
import std;

namespace async_simple {

namespace coro {

class CountEvent;

namespace detail {

// CountEvent is a count-down event.
// The last 'down' will resume the awaiting coroutine on this event.
class CountEvent {
public:
    CountEvent(std::size_t count) : _count(count + 1) {}
    CountEvent(const CountEvent&) = delete;
    CountEvent(CountEvent&& other)
        : _count(other._count.exchange(0, std::memory_order_relaxed)),
          _awaitingCoro(std::exchange(other._awaitingCoro, nullptr)) {}

    [[nodiscard]] CoroHandle<> down(std::size_t n = 1) {
        // read acquire and write release, _awaitingCoro store can not be
        // reordered after this barrier
        auto oldCount = _count.fetch_sub(n, std::memory_order_acq_rel);
        if (oldCount == 1) {
            auto awaitingCoro = _awaitingCoro;
            _awaitingCoro = nullptr;
            return awaitingCoro;
        } else {
            return nullptr;
            // return nullptr instead of noop_coroutine could save one time
            // for accessing the memory.
            // return std::experimental::noop_coroutine();
        }
    }
    [[nodiscard]] std::size_t downCount(std::size_t n = 1) {
        // read acquire and write release
        return _count.fetch_sub(n, std::memory_order_acq_rel);
    }

    void setAwaitingCoro(CoroHandle<> h) { _awaitingCoro = h; }

private:
    std::atomic<std::size_t> _count;
    CoroHandle<> _awaitingCoro;
};

}  // namespace detail

}  // namespace coro
}  // namespace async_simple
