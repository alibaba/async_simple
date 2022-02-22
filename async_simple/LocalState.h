
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
#ifndef ASYNC_SIMPLE_LOCALSTATE_H
#define ASYNC_SIMPLE_LOCALSTATE_H

#include <atomic>

#include <async_simple/Common.h>
#include <async_simple/Executor.h>
#include <async_simple/MoveWrapper.h>
#include <async_simple/Try.h>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

namespace async_simple {

// A component of Future/Promise. LocalState is owned by
// Future only. LocalState should be valid after
// Future and Promise disconnects.
//
// Users should never use LocalState directly.
template <typename T>
class LocalState {
private:
    using Continuation = std::function<void(Try<T>&& value)>;

public:
    LocalState() : _executor(nullptr) {}
    LocalState(T&& v) : _try(std::forward<T>(v)), _executor(nullptr) {}
    LocalState(Try<T>&& t) : _try(std::move(t)), _executor(nullptr) {}

    ~LocalState() {}

    LocalState(const LocalState&) = delete;
    LocalState& operator=(const LocalState&) = delete;

    LocalState(LocalState&& other)
        : _try(std::move(other._try)),
          _executor(std::exchange(other._executor, nullptr)) {}
    LocalState& operator=(LocalState&& other) {
        if (this != &other) {
            std::swap(_try, other._try);
            std::swap(_executor, other._executor);
        }
        return *this;
    }

public:
    bool hasResult() const noexcept { return _try.available(); }

public:
    Try<T>& getTry() noexcept { return _try; }
    const Try<T>& getTry() const noexcept { return _try; }

    void setExecutor(Executor* ex) { _executor = ex; }

    Executor* getExecutor() { return _executor; }

    bool currentThreadInExecutor() const {
        if (!_executor) {
            return false;
        }
        return _executor->currentThreadInExecutor();
    }

    template <typename F>
    void setContinuation(F&& f) {
        assert(_try.available());
        std::forward<F>(f)(std::move(_try));
    }

private:
    Try<T> _try;
    Executor* _executor;
};
}  // namespace async_simple

#endif
