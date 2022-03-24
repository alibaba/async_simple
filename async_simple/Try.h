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

#include <async_simple/Common.h>
#include <async_simple/Unit.h>
#include <cassert>
#include <exception>
#include <variant>

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
    Try() : _result{} {}
    ~Try() { destroy(); }

    Try(Try<T>&& other) : _result(std::move(other._result)) {
    }
    template <typename T2 = T>
    Try(std::enable_if_t<std::is_same<Unit, T2>::value, const Try<void>&>
            other) {
        if (other.hasError()) {
            _result = other._error;
        } else {
            _result = T();
        }
    }
    Try& operator=(Try<T>&& other) {
        if (&other == this) {
            return *this;
        }

        destroy();

        _result = std::move(other._result);
        return *this;
    }
    Try& operator=(std::exception_ptr error) {
        if (_result.index() == 2 && error == std::get<2>(_result)) {
            return *this;
        }

        destroy();

        _result = error;
        return *this;
    }

    Try(const T& val) : _result(val) {}
    Try(T&& val) : _result(std::move(val)) {}
    Try(std::exception_ptr error)
        : _result(error) {}

private:
    Try(const Try&) = delete;
    Try& operator=(const Try&) = delete;

public:
    bool available() const { return _result.index() != 0; }
    bool hasError() const { return _result.index() == 2; }
    const T& value() const& {
        checkHasTry();
        return std::get<1>(_result);
    }
    T& value() & {
        checkHasTry();
        return std::get<1>(_result);
    }
    T&& value() && {
        checkHasTry();
        return std::move(std::get<1>(_result));
    }
    const T&& value() const&& {
        checkHasTry();
        return std::move(std::get<1>(_result));
    }

    void setException(std::exception_ptr error) {
        if (_result.index() == 2 && std::get<2>(_result) == error) {
            return;
        }
        destroy();
        _result = error;
    }
    std::exception_ptr getException() {
        logicAssert(_result.index() == 2,
                    "Try object do not has an error");
        return std::get<2>(_result);
    }

private:
    FL_INLINE void checkHasTry() const {
        if (FL_LIKELY(_result.index() == 1)) {
            return;
        } else if (_result.index() == 2) {
            std::rethrow_exception(std::get<2>(_result));
        } else if (_result.index() == 0) {
            throw std::logic_error("Try object is empty");
        } else {
            assert(false);
        }
    }

    void destroy() {
        _result = {};
    }

private:
    std::variant<std::monostate, T, std::exception_ptr> _result;

private:
    friend Try<Unit>;
};

template <>
class Try<void> {
public:
    Try() {}
    Try(std::exception_ptr error) : _error(error) {}

    Try& operator=(std::exception_ptr error) {
        _error = error;
        return *this;
    }

public:
    Try(Try&& other) : _error(std::move(other._error)) {}
    Try& operator=(Try&& other) {
        if (this != &other) {
            std::swap(_error, other._error);
        }
        return *this;
    }

public:
    void value() {
        if (_error) {
            std::rethrow_exception(_error);
        }
    }

    bool hasError() const { return _error.operator bool(); }

    void setException(std::exception_ptr error) { _error = error; }
    std::exception_ptr getException() { return _error; }

private:
    std::exception_ptr _error;

private:
    friend Try<Unit>;
};

// T is Non void
template <typename F, typename... Args>
std::enable_if_t<!(std::is_same<std::invoke_result_t<F, Args...>, void>::value),
                 Try<std::invoke_result_t<F, Args...>>>
makeTryCall(F&& f, Args&&... args) {
    using T = std::invoke_result_t<F, Args...>;
    try {
        return Try<T>(std::forward<F>(f)(std::forward<Args>(args)...));
    } catch (...) {
        return Try<T>(std::current_exception());
    }
}

// T is void
template <typename F, typename... Args>
std::enable_if_t<std::is_same<std::invoke_result_t<F, Args...>, void>::value,
                 Try<void>>
makeTryCall(F&& f, Args&&... args) {
    try {
        std::forward<F>(f)(std::forward<Args>(args)...);
        return Try<void>();
    } catch (...) {
        return Try<void>(std::current_exception());
    }
}

}  // namespace async_simple

#endif  // ASYNC_SIMPLE_TRY_H
