/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
#ifndef ASYNC_SIMPLE_TRY_H
#define ASYNC_SIMPLE_TRY_H

#include <cassert>
#include <exception>
#include <functional>
#include <new>
#include <utility>
#include <variant>
#include "async_simple/Common.h"

namespace async_simple {

// Try<T> contains either an instance of T, an exception, or nothing.
// Try<T>::value() will return the instance.
// If exception or nothing inside, Try<T>::value() would throw an exception.
//
// You can create a Try<T> by:
// 1. default constructor: contains nothing.
// 2. construct from exception_ptr.
// 3. construct from value T.
// 4. moved from another Try<T> instance.
template <typename T>
class Try {
public:
    Try() = default;
    ~Try() = default;
    Try(Try<T>&&) = default;
    Try& operator=(Try&& other) = default;

    template <class U>
    Try& operator=(U&& value) requires std::is_convertible_v<U, T> {
        _value.template emplace<T>(std::forward<U>(value));
        return *this;
    }

    Try& operator=(std::exception_ptr error) {
        _value.template emplace<std::exception_ptr>(error);
        return *this;
    }

    template <class U>
    Try(U&& value) requires std::is_convertible_v<U, T> {
        _value.template emplace<T>(std::forward<U>(value));
    }

    Try(std::exception_ptr error) {
        _value.template emplace<std::exception_ptr>(error);
    }

private:
    Try(const Try&) = delete;
    Try& operator=(const Try&) = delete;

public:
    constexpr bool available() const noexcept { return _value.index() != 0; }
    constexpr bool hasError() const noexcept { return _value.index() == 2; }
    const T& value() const& {
        checkHasTry();
        return std::get<1>(_value);
    }
    T& value() & {
        checkHasTry();
        return std::get<1>(_value);
    }
    T&& value() && {
        checkHasTry();
        return std::move(std::get<1>(_value));
    }
    const T&& value() const&& {
        checkHasTry();
        return std::move(std::get<1>(_value));
    }

    template <class U>
    constexpr T& emplace(U&& value) requires std::is_convertible_v<U, T> {
        return _value.template emplace<T>(std::forward<U>(value));
    }

    void setException(std::exception_ptr error) {
        _value.template emplace<std::exception_ptr>(error);
    }

    std::exception_ptr getException() {
        logicAssert(_value.index() == 2, "Try object do not has on error");
        return std::get<2>(_value);
    }

private:
    AS_INLINE void checkHasTry() const {
        switch (_value.index()) {
            case 0:
                throw std::logic_error("Try object is empty");
            case 1:
                return;
            case 2:
                std::rethrow_exception(std::get<2>(_value));
            default:
                assert(false);
        }
    }

private:
    std::variant<std::monostate, T, std::exception_ptr> _value;
};

template <>
class Try<void> {
public:
    Try(bool hasValue = false) : _hasValue(hasValue) {}
    Try(std::exception_ptr error) : _error(error), _hasValue(false) {}

    Try& operator=(std::exception_ptr error) {
        _error = error;
        return *this;
    }

public:
    Try(Try&& other)
        : _error(std::move(other._error)), _hasValue(other._hasValue) {}
    Try& operator=(Try&& other) {
        if (this != &other) {
            std::swap(_error, other._error);
            std::swap(_hasValue, other._hasValue);
        }
        return *this;
    }

public:
    void value() {
        if (_error) {
            std::rethrow_exception(_error);
        }
    }
    constexpr void emplace() { _hasValue = true; }
    constexpr bool available() const noexcept {
        return _hasValue || hasError();
    }
    bool hasError() const { return _error.operator bool(); }

    void setException(std::exception_ptr error) { _error = error; }
    std::exception_ptr getException() { return _error; }

private:
    std::exception_ptr _error;
    bool _hasValue;
};

// T is Non void
template <typename F, typename... Args>
auto makeTryCall(F&& f, Args&&... args) {
    using T = std::invoke_result_t<F, Args...>;
    try {
        if constexpr (std::is_void_v<T>) {
            std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            return Try<void>(true);
        } else {
            return Try<T>(
                std::invoke(std::forward<F>(f), std::forward<Args>(args)...));
        }
    } catch (...) {
        return Try<T>(std::current_exception());
    }
}

}  // namespace async_simple

#endif  // ASYNC_SIMPLE_TRY_H
