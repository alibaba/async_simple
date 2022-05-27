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
module async_simple:coro.Task;

import experimental.coroutine;
import std;

namespace async_simple {

class Executor;

namespace coro {
namespace detail {

template <typename T>
struct TaskPromise;

}

// Task is a fake coroutine. It never suspends and gets resumed.
// This is used for testing the performance with real coroutine, Lazy.
template <typename T>
class [[nodiscard]] Task {
public:
    using promise_type = detail::TaskPromise<T>;
    using Handle = std::experimental::coroutine_handle<promise_type>;
    struct Awaiter {
        constexpr bool await_ready() const noexcept { return true; }
        void await_suspend(
            std::experimental::coroutine_handle<> continuation) const noexcept {
        }
        inline __attribute__((__always_inline__))  T await_resume() noexcept {
            return std::move(task->stealValue());
        }

        Task* task;

        Awaiter(Task* t) : task(t) {}
    };

public:
    Task(Handle coro) : _coro(coro), _hasValue(false) {
        coro.promise().task = this;
    }
    ~Task() {
        if (_hasValue) [[unlikely]] {
            _value.~T();
        }
    }
    Task(Task && other)
        : _coro(std::exchange(other._coro, nullptr)),
          _hasValue(std::exchange(other._hasValue, false)) {
        new (std::addressof(_value)) T(std::move(other._value));
        _coro.promise().task = this;
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task& operator=(Task&&) = delete;

public:
    auto coAwait(Executor * ex) { return Awaiter(this); }
    auto operator co_await() { return Awaiter(this); }

private:
    template <typename C>
    inline __attribute__((__always_inline__))  void setValue(C && v) noexcept {
        new (std::addressof(_value)) T(std::forward<C>(v));
        _hasValue = true;
    }

    inline __attribute__((__always_inline__))  T&& stealValue() noexcept {
        _hasValue = false;
        return std::move(_value);
    }

private:
    Handle _coro;
    union {
        T _value;
    };
    bool _hasValue;

private:
    friend struct detail::TaskPromise<T>;
};

template <>
class [[nodiscard]] Task<void> {
public:
    using promise_type = detail::TaskPromise<void>;
    using Handle = std::experimental::coroutine_handle<promise_type>;
    struct Awaiter {
        constexpr bool await_ready() const noexcept { return true; }
        void await_suspend(
            std::experimental::coroutine_handle<> continuation) const noexcept {
        }
        inline __attribute__((__always_inline__))  void await_resume() const noexcept {}
        Awaiter() = default;
    };

public:
    Task() = default;
    Task(Task &&) = default;

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task& operator=(Task&&) = delete;

public:
    auto coAwait(Executor * ex) { return Awaiter(); }
    auto operator co_await() { return Awaiter(); }
};

namespace detail {

struct TaskPromiseBase {
    constexpr std::experimental::suspend_never initial_suspend() const
        noexcept {
        return {};
    }
    constexpr std::experimental::suspend_never final_suspend() const noexcept {
        return {};
    }
    void unhandled_exception() const noexcept {}

    void* operator new(std::size_t sz) noexcept { return ::operator new(sz); }
    void operator delete(void* p, std::size_t sz) noexcept {
        return ::operator delete(p);
    }
};

template <typename T>
struct TaskPromise : public TaskPromiseBase {
    inline __attribute__((__always_inline__))  void return_value(T&& v) { task->setValue(std::move(v)); }
    inline __attribute__((__always_inline__))  void return_value(const T& v) { task->setValue(v); }

    TaskPromise() : task(nullptr) {}

    Task<T> get_return_object() noexcept {
        return Task<T>(Task<T>::Handle::from_promise(*this));
    }

    Task<T>* task;
};

template <>
struct TaskPromise<void> : public TaskPromiseBase {
    inline __attribute__((__always_inline__))  void return_void() const noexcept {}
    Task<void> get_return_object() noexcept { return Task<void>(); }
};

template <typename T, bool Ready>
struct CoroTypeTraits {};

template <typename T>
struct CoroTypeTraits<T, true> {
    using TaskType = Task<T>;
};

}  // namespace detail

}  // namespace coro
}  // namespace async_simple
