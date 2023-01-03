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
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>

#include "async_simple/Common.h"
#include "async_simple/Try.h"
#include "async_simple/experimental/coroutine.h"

namespace async_simple::coro {

template <class Ref, class V = void, class Allocator = void>
class Generator {
private:
    using value =
        std::conditional_t<std::is_void_v<V>, std::remove_cvref_t<Ref>, V>;
    using reference = std::conditional_t<std::is_void_v<V>, Ref&&, Ref>;

    class iterator;

    // clang-format off
    static_assert(
        std::same_as<std::remove_cvref_t<value>, value> &&
            std::is_object_v<value>,
        "generator's value type must be a cv-unqualified object type");
    static_assert(std::is_reference_v<reference> ||
                      (std::is_object_v<reference> &&
                       std::same_as<std::remove_cv_t<reference>, reference> &&
                       std::copy_constructible<reference>),
                  "generator's second argument must be a reference type or a "
                  "cv-unqualified "
                  "copy-constructible object type");
    // clang-format on

public:
    class promise_type;

    using Handle = std::coroutine_handle<promise_type>;
    using yielded = std::conditional_t<std::is_reference_v<reference>,
                                       reference, const reference&>;

    Generator(Generator&& other) noexcept
        : _coro(std::exchange(other._coro, nullptr)) {}
    Generator& operator=(Generator&& other) noexcept;
    ~Generator();
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    [[nodiscard]] iterator begin();
    [[nodiscard]] std::default_sentinel_t end() const noexcept { return {}; }

private:
    explicit Generator(Handle coro) noexcept;

private:
    Handle _coro = nullptr;
};

template <class Ref, class V, class Allocator>
class Generator<Ref, V, Allocator>::promise_type {
private:
    struct elementAwaiter {
        std::remove_cvref_t<yielded> value_;
        constexpr bool await_ready() const noexcept { return false; }

        template <class Promise>
        constexpr void await_suspend(
            std::coroutine_handle<Promise> handle) noexcept {
            auto& current = handle.promise();
            current.value_ = std::addressof(value_);
        }

        constexpr void await_resume() const noexcept {}
    };

public:
    // Unlike `Lazy`, Generator will run directly when it is created until it is
    // suspended for the first time
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }

    std::suspend_always yield_value(yielded val) noexcept {
        value_ = std::addressof(val);
        return {};
    }

    // clang-format off
    auto yield_value(const std::remove_reference_t<yielded>& lval) requires
        std::is_rvalue_reference_v<yielded> && std::constructible_from<
        std::remove_cvref_t<yielded>, const std::remove_reference_t<yielded>& > 
    { return elementAwaiter{lval}; }
    // clang-format on

    void return_void() noexcept {}
    void await_transform() = delete;
    void unhandled_exception() noexcept { except_ = std::current_exception(); }

    Generator get_return_object() noexcept {
        return Generator(Generator::Handle::from_promise(*this));
    }

    std::exception_ptr getException() const { return except_; }

    friend class Generator;
    friend class Generator::iterator;

private:
    std::add_pointer_t<yielded> value_;
    std::exception_ptr except_;
};

template <class Ref, class V, class Allocator>
class Generator<Ref, V, Allocator>::iterator {
public:
    using value_type = value;
    using difference_type = std::ptrdiff_t;

    explicit iterator(Handle coro) : _coro(coro) { checkFinished(); }
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

    [[nodiscard]] bool operator==(std::default_sentinel_t) const {
        return !_coro || _coro.done();
    }

    reference operator*() const {
        logicAssert(
            this->operator bool(),
            "Should have a coroutine handle or the coroutine has not ended");
        return static_cast<yielded>(*_coro.promise().value_);
    }

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

    Handle _coro = nullptr;
};

template <class Ref, class V, class Allocator>
Generator<Ref, V, Allocator>::Generator(Handle coro) noexcept : _coro(coro) {}

template <class Ref, class V, class Allocator>
Generator<Ref, V, Allocator>& Generator<Ref, V, Allocator>::operator=(
    Generator&& other) noexcept {
    if (_coro) {
        if (!_coro.done()) {
            // TODO: [Warning] the coroutine is not done!
        }
        _coro.destroy();
    }
    _coro = std::exchange(other._coro, nullptr);
    return *this;
}

template <class Ref, class V, class Allocator>
Generator<Ref, V, Allocator>::~Generator() {
    if (_coro) {
        if (!_coro.done()) {
            // TODO: log
        }
        _coro.destroy();
        _coro = nullptr;
    }
}

template <class Ref, class V, class Allocator>
typename Generator<Ref, V, Allocator>::iterator
Generator<Ref, V, Allocator>::begin() {
    return iterator(std::exchange(_coro, nullptr));
}

}  // namespace async_simple::coro

template <class Ref, class V, class Allocator>
inline constexpr bool
    std::ranges::enable_view<async_simple::coro::Generator<Ref, V, Allocator>> =
        true;

#endif
