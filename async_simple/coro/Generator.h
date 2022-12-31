/*
 * Copyright (c) 2023, Alibaba Group Holding Limited;
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

#ifndef ASYNC_SIMPLE_CORO_GENERATOR_H
#define ASYNC_SIMPLE_CORO_GENERATOR_H

#include <concepts>
#include <exception>
#include <ranges>
#include <utility>

#include "async_simple/Common.h"
#include "async_simple/Try.h"
#include "async_simple/experimental/coroutine.h"

namespace async_simple::coro {

template <typename T>
class Generator {
public:
    class promise_type;
    class iterator;

    using Handle = std::coroutine_handle<promise_type>;

    explicit Generator(Handle coro) noexcept;
    Generator(Generator&& other) noexcept
        : _coro(std::exchange(other._coro, nullptr)) {}
    Generator& operator=(Generator&& other) noexcept;
    ~Generator();
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    //
    explicit operator bool() const noexcept;

    //
    T operator()() const;
    //
    iterator begin();

    //
    std::default_sentinel_t end() const noexcept { return {}; }

private:
    Handle _coro = nullptr;
};

template <typename T>
class Generator<T>::promise_type {
public:
    // Unlike `Lazy`, Generator will run directly when it is created until it is
    // suspended for the first time
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }

    template <std::convertible_to<T> From>
    std::suspend_always yield_value(From&& from) {
        _value.emplace(std::forward<From>(from));
        return {};
    }

    void return_void() noexcept {}
    void unhandled_exception() noexcept {
        _value.setException(std::current_exception());
    }

    Generator get_return_object() noexcept {
        return Generator(Generator::Handle::from_promise(*this));
    }

    std::exception_ptr getException() const {
        return _value.hasError() ? _value.getException() : nullptr;
    }

    friend class Generator;
    friend class Generator::iterator;

private:
    Try<T> _value;
};

template <typename T>
class Generator<T>::iterator {
public:
    using value_type = T;
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    friend class Generator;

    explicit iterator() = default;
    ~iterator() {
        if (_coro) {
            // TODO: error log

            _coro.destroy();
            _coro = nullptr;
        }
    }
    iterator(iterator&& rhs) noexcept
        : _coro(std::exchange(rhs._coro, nullptr)) {}

    iterator& operator=(iterator&& rhs) {
        logicAssert(!_coro, "Should not own a coroutine handle");
        _coro = std::exchange(rhs._coro, nullptr);
    }

    explicit operator bool() const noexcept { return _coro && !_coro.done(); }

    bool operator==(std::default_sentinel_t) const {
        return !_coro || _coro.done();
    }
    bool operator==(const iterator& rhs) const { return _coro == rhs._coro; }
    bool operator!=(const iterator& rhs) const { return _coro != rhs._coro; }

    reference operator*() const {
        logicAssert(
            this->operator bool(),
            "Should have a coroutine handle or the coroutine has not ended");
        return _coro.promise()._value.value();
    }
    pointer operator->() const { return std::addressof(operator*()); }

    iterator& operator++() {
        if (_coro) {
            _coro.resume();
            checkFinished();
        }
        return *this;
    }

    void operator++(int) { ++(*this); }

private:
    // Check if the Generator is complete, if not, do nothing,
    // if so, destroy the coroutine handle and set it to `nullptr`,
    // if there is an exception, throw an exception
    void checkFinished() {
        if (_coro && _coro.done()) {
            auto exception = _coro.promise().getException();

            _coro.destroy();
            _coro = nullptr;

            if (exception) {
                std::rethrow_exception(exception);
            }
        }
    }

    explicit iterator(Handle coro) : _coro(coro) { checkFinished(); }

    Handle _coro = nullptr;
};

template <typename T>
Generator<T>::Generator(Handle coro) noexcept : _coro(coro) {}

template <typename T>
Generator<T>& Generator<T>::operator=(Generator&& other) noexcept {
    if (_coro) {
        if (!_coro.done()) {
            // TODO: [Warning] the coroutine is not done!
        }
        _coro.destroy();
    }
    _coro = std::exchange(other._coro, nullptr);
    return *this;
}

template <typename T>
Generator<T>::~Generator() {
    if (_coro) {
        if (!_coro.done()) {
            // TODO: log
        }
        _coro.destroy();
        _coro = nullptr;
    }
}

template <typename T>
Generator<T>::operator bool() const noexcept {
    return _coro &&
           (!_coro.done() ||
            /* If there is an exception, then return true,
                the purpose is to call operator () to throw an exception
            */
            _coro.promise()._value.hasError());
}

template <typename T>
T Generator<T>::operator()() const {
    logicAssert(!!_coro, "couroutine handle is nullptr");
    auto res = std::move(_coro.promise()._value.value());
#ifdef __APPLE__
    // FIXME: `std::coroutine_handle.resume()` is a non-const member function in
    // macos
    const_cast<Generator<T>&>(*this)._coro();
#else
    _coro.resume();
#endif
    return res;
}

template <typename T>
typename Generator<T>::iterator Generator<T>::begin() {
    return iterator(std::exchange(_coro, nullptr));
}

}  // namespace async_simple::coro

template <class T>
inline constexpr bool
    std::ranges::enable_view<async_simple::coro::Generator<T>> = true;

#endif
