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
/* This file implements a simple condition variable. This is used as a
 * low level component in async_simple. Users shouldn't use this directly.
 */
#ifndef ASYNC_SIMPLE_UTIL_CONDITION_H
#define ASYNC_SIMPLE_UTIL_CONDITION_H

#include <async_simple/Common.h>
#include <exception>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#include <atomic>

namespace async_simple {
namespace util {

inline int32_t futexWake(int32_t* uaddr, int32_t n) {
    return syscall(__NR_futex, uaddr, FUTEX_WAKE, n, nullptr, nullptr, 0);
}

inline int32_t futexWait(int32_t* uaddr, int32_t val) {
    return syscall(__NR_futex, uaddr, FUTEX_WAIT, val, nullptr, nullptr, 0);
}

class Condition {
public:
    Condition() : _value(0) {}

public:
    void set();
    void wait();

private:
    std::atomic<int32_t> _value;
};

inline void Condition::set() {
    _value.store(1, std::memory_order_release);
    [[maybe_unused]] auto ret = futexWake(reinterpret_cast<int32_t*>(&_value),
                                          std::numeric_limits<int32_t>::max());
}

inline void Condition::wait() {
    auto oldValue = _value.load(std::memory_order_acquire);
    while (oldValue == 0) {
        auto ret = futexWait(reinterpret_cast<int32_t*>(&_value), 0);
        if (ret != 0) {
            logicAssert(errno == EAGAIN || errno == EINTR ||
                            errno == ETIMEDOUT || errno == EWOULDBLOCK,
                        "futex wait error");
        }
        oldValue = _value.load(std::memory_order_acquire);
    }
}

}  // namespace util
}  // namespace async_simple

#endif
