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
#ifndef ASYNC_SIMPLE_CORO_LAZY_H
#define ASYNC_SIMPLE_CORO_LAZY_H

#include <async_simple/Common.h>
#include <async_simple/Try.h>
#include <async_simple/coro/DetachedCoroutine.h>
#include <async_simple/coro/ViaCoroutine.h>
#include <async_simple/experimental/coroutine.h>
#include <atomic>
#include <cstdio>
#include <exception>

namespace async_simple {

class Executor;

namespace coro {

template <typename T>
class Lazy;

// In the middle of the execution of one coroutine, if we want to give out the
// rights to execute back to the executor, to make it schedule other tasks to
// execute, we could write:
//
// ```C++
//  co_await Yield();
// ```
//
// This would suspend the executing coroutine.
struct Yield {};

namespace detail {
template <typename LazyType, typename IAlloc, typename OAlloc, bool Para>
struct CollectAllAwaiter;

template <typename LazyType, typename IAlloc>
struct CollectAnyAwaiter;

template <template <typename> typename LazyType, typename... Ts>
struct CollectAnyVariadicAwaiter;

}  // namespace detail

namespace detail {

class LazyPromiseBase {
public:
    // Resume the caller waiting to the current coroutine. Note that we need
    // destroy the frame for the current coroutine explicitly. Since after
    // FinalAwaiter, The current coroutine should be suspended and never to
    // resume. So that we couldn't expect it to release it self any more.
    struct FinalAwaiter {
        bool await_ready() const noexcept { return false; }
        template <typename PromiseType>
        auto await_suspend(std::coroutine_handle<PromiseType> h) noexcept {
            return h.promise()._continuation;
        }
        void await_resume() noexcept {}
    };

    struct YieldAwaiter {
        YieldAwaiter(Executor* executor) : _executor(executor) {}
        bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> handle) {
            std::function<void()> func = [h = std::move(handle)]() mutable {
                h.resume();
            };
            _executor->schedule(func);
        }
        void await_resume() noexcept {}

    private:
        Executor* _executor;
    };

public:
    LazyPromiseBase() : _executor(nullptr) {}
    // Lazily started, coroutine will not execute until first resume() is called
    std::suspend_always initial_suspend() noexcept { return {}; }
    FinalAwaiter final_suspend() noexcept { return {}; }

    template <typename Awaitable>
    auto await_transform(Awaitable&& awaitable) {
        // See CoAwait.h for details.
        return detail::coAwait(_executor, std::forward<Awaitable>(awaitable));
    }

    auto await_transform(CurrentExecutor) {
        return ReadyAwaiter<Executor*>(_executor);
    }
    auto await_transform(Yield) { return YieldAwaiter(_executor); }

    /// IMPORTANT: _continuation should be the first member due to the
    /// requirement of dbg script.
    std::coroutine_handle<> _continuation;
    Executor* _executor;
};

template <typename T>
class LazyPromise : public LazyPromiseBase {
public:
    LazyPromise() noexcept {}
    ~LazyPromise() noexcept {
        switch (_resultType) {
            case result_type::value:
                _value.~T();
                break;
            case result_type::exception:
                _exception.~exception_ptr();
                break;
            default:
                break;
        }
    }

    Lazy<T> get_return_object() noexcept;

    template <typename V,
              typename = std::enable_if_t<std::is_convertible_v<V&&, T>>>
    void return_value(V&& value) noexcept(
        std::is_nothrow_constructible_v<T, V&&>) {
        ::new (static_cast<void*>(std::addressof(_value)))
            T(std::forward<V>(value));
        _resultType = result_type::value;
    }
    void unhandled_exception() noexcept {
        ::new (static_cast<void*>(std::addressof(_exception)))
            std::exception_ptr(std::current_exception());
        _resultType = result_type::exception;
    }

public:
    T& result() & {
        if (_resultType == result_type::exception)
            AS_UNLIKELY { std::rethrow_exception(_exception); }
        assert(_resultType == result_type::value);
        return _value;
    }
    T&& result() && {
        if (_resultType == result_type::exception)
            AS_UNLIKELY { std::rethrow_exception(_exception); }
        assert(_resultType == result_type::value);
        return std::move(_value);
    }

    Try<T> tryResult() noexcept {
        if (_resultType == result_type::exception)
            AS_UNLIKELY { return Try<T>(_exception); }
        else {
            assert(_resultType == result_type::value);
            return Try<T>(std::move(_value));
        }
    }

    enum class result_type { empty, value, exception };
    result_type _resultType = result_type::empty;
    union {
        T _value;
        std::exception_ptr _exception;
    };
};

template <>
class LazyPromise<void> : public LazyPromiseBase {
public:
    Lazy<void> get_return_object() noexcept;
    void return_void() noexcept {}
    void unhandled_exception() noexcept {
        _exception = std::current_exception();
    }

    void result() {
        if (_exception != nullptr)
            AS_UNLIKELY { std::rethrow_exception(_exception); }
    }
    Try<void> tryResult() noexcept { return Try<void>(_exception); }

public:
    std::exception_ptr _exception{nullptr};
};

}  // namespace detail

template <typename T>
class RescheduleLazy;

namespace detail {

template <typename T>
struct LazyAwaiterBase {
    using Handle = CoroHandle<detail::LazyPromise<T>>;
    Handle _handle;

    LazyAwaiterBase(LazyAwaiterBase& other) = delete;
    LazyAwaiterBase& operator=(LazyAwaiterBase& other) = delete;

    LazyAwaiterBase(LazyAwaiterBase&& other)
        : _handle(std::exchange(other._handle, nullptr)) {}

    LazyAwaiterBase& operator=(LazyAwaiterBase&& other) {
        std::swap(_handle, other._handle);
        return *this;
    }

    LazyAwaiterBase(Handle coro) : _handle(coro) {}
    ~LazyAwaiterBase() {
        if (_handle) {
            _handle.destroy();
            _handle = nullptr;
        }
    }

    bool await_ready() const noexcept { return false; }

    template <typename T2 = T, std::enable_if_t<std::is_void_v<T2>, int> = 0>
    void awaitResume() {
        _handle.promise().result();
        // We need to destroy the handle expclictly since the awaited coroutine
        // after symmetric transfer couldn't release it self any more.
        _handle.destroy();
        _handle = nullptr;
    }

    template <typename T2 = T, std::enable_if_t<!std::is_void_v<T2>, int> = 0>
    T awaitResume() {
        auto r = std::move(_handle.promise()).result();
        _handle.destroy();
        _handle = nullptr;
        return r;
    }

    Try<T> awaitResumeTry() noexcept {
        Try<T> ret = std::move(_handle.promise().tryResult());
        _handle.destroy();
        _handle = nullptr;
        return ret;
    }
};

}  // namespace detail

// Lazy is a coroutine task which would be executed lazily.
// The user who wants to use Lazy should declare a function whose return type
// is Lazy<T>. T is the type you want the function to return originally.
// And if the function doesn't want to return any thing, use Lazy<>.
//
// Then in the function, use co_return instead of return. And use co_await to
// wait things you want to wait. For example:
//
// ```C++
//  // Return 43 after 10s.
//  Lazy<int> foo() {
//     co_await sleep(10s);
//     co_return 43;
// }
// ```
//
// To get the value wrapped in Lazy, we could co_await it like:
//
// ```C++
//  Lazy<int> bar() {
//      // This would return the value foo returned.
//      co_return co_await foo();
// }
// ```
//
// If we don't want the caller to be a coroutine too, we could use Lazy::start
// to get the value asynchronously.
//
// ```C++
// void foo_use() {
//     foo().start([](Try<int> &&value){
//         std::cout << "foo: " << value.value() << "\n";
//     });
// }
// ```
//
// When the foo gets its value, the value would be passed to the lambda in
// Lazy::start().
//
// If the user wants to get the value synchronously, he could use
// async_simple::coro::syncAwait.
//
// ```C++
// void foo_use2() {
//     auto val = async_simple::coro::syncAwait(foo());
//     std::cout << "foo: " << val << "\n";
// }
// ```
//
// There is no executor instance in a Lazy. To specify an executor to schedule
// the execution of the Lazy and corresponding Lazy tasks inside, user could use
// `Lazy::via` to assign an executor for this Lazy. `Lazy::via` would return a
// RescheduleLazy. User should use the returned RescheduleLazy directly. The
// Lazy which called `via()` shouldn't be used any more.
//
// If Lazy is co_awaited directly, sysmmetric transfer would happend. That is,
// the stack frame for current caller would be released and the lazy task would
// be resumed directly. So the user needn't to worry about the stack overflow.
//
// The co_awaited Lazy shouldn't be accessed any more.
//
// When a Lazy is co_awaited, if there is any exception happened during the
// process, the co_awaited expression would throw the exception happened. If the
// user does't want the co_await expression to throw an exception, he could use
// `Lazy::coAwaitTry`. For example:
//
//  ```C++
//      Try<int> res = co_await foo().coAwaitTry();
//      if (res.hasError())
//          std::cout << "Error happend.\n";
//      else
//          std::cout << "We could get the value: " << res.value() << "\n";
// ```
//
// If any awaitable wants to derive the executor instance from its caller, it
// should implement `coAwait(Executor*)` member method. Then the caller would
// pass its executor instance to the awaitable.
template <typename T = void>
class [[nodiscard]] Lazy {
public:
    using promise_type = detail::LazyPromise<T>;
    using Handle = CoroHandle<promise_type>;
    using ValueType = T;

    struct AwaiterBase : public detail::LazyAwaiterBase<T> {
        using Base = detail::LazyAwaiterBase<T>;
        AwaiterBase(Handle coro) : Base(coro) {}

        AS_INLINE std::coroutine_handle<> await_suspend(
            std::coroutine_handle<> continuation) noexcept {
            // current coro started, caller becomes my continuation
            Base::_handle.promise()._continuation = continuation;
            return Base::_handle;
        }
    };

    struct TryAwaiter : public AwaiterBase {
        TryAwaiter(Handle coro) : AwaiterBase(coro) {}
        AS_INLINE Try<T> await_resume() noexcept {
            return AwaiterBase::awaitResumeTry();
        };
    };

    struct ValueAwaiter : public AwaiterBase {
        ValueAwaiter(Handle coro) : AwaiterBase(coro) {}
        AS_INLINE T await_resume() { return AwaiterBase::awaitResume(); }
    };

    ~Lazy() {
        if (_coro) {
            _coro.destroy();
            _coro = nullptr;
        }
    };
    explicit Lazy(Handle coro) : _coro(coro) {}
    Lazy(Lazy&& other) : _coro(std::move(other._coro)) {
        other._coro = nullptr;
    }

    Lazy(const Lazy&) = delete;
    Lazy& operator=(const Lazy&) = delete;

    // Bind an executor to a Lazy, and convert it to RescheduleLazy.
    // You can only call via on rvalue, i.e. a Lazy is not accessible after
    // via() called.
    RescheduleLazy<T> via(Executor* ex) && {
        logicAssert(_coro.operator bool(),
                    "Lazy do not have a coroutine_handle");
        _coro.promise()._executor = ex;
        return RescheduleLazy<T>(std::exchange(_coro, nullptr));
    }

    Executor* getExecutor() { return _coro.promise()._executor; }

    // Bind an executor only. Don't re-schedule.
    //
    // Users shouldn't use `setEx` directly. `setEx` is designed
    // for internal purpose only. See uthread/Await.h/await for details.
    Lazy<T> setEx(Executor* ex) && {
        logicAssert(_coro.operator bool(),
                    "Lazy do not have a coroutine_handle");
        _coro.promise()._executor = ex;
        return Lazy<T>(std::exchange(_coro, nullptr));
    }

    template <typename F>
    void start(F&& callback) {
        // callback should take a single Try<T> as parameter, return value will
        // be ignored. a detached coroutine will not suspend at initial/final
        // suspend point.
        auto launchCoro = [](Lazy lazy,
                             std::decay_t<F> cb) -> detail::DetachedCoroutine {
            cb(std::move(co_await lazy.coAwaitTry()));
        };
        [[maybe_unused]] auto detached =
            launchCoro(std::move(*this), std::forward<F>(callback));
    }

    bool isReady() const { return !_coro || _coro.done(); }

    auto operator co_await() {
        return ValueAwaiter(std::exchange(_coro, nullptr));
    }

    auto coAwait(Executor* ex) {
        // derived lazy inherits executor
        _coro.promise()._executor = ex;
        return ValueAwaiter(std::exchange(_coro, nullptr));
    }
    auto coAwaitTry() { return TryAwaiter(std::exchange(_coro, nullptr)); }

private:
    Handle _coro;
    friend class RescheduleLazy<T>;

    template <typename LazyType, typename IAlloc, typename OAlloc, bool Para>
    friend struct detail::CollectAllAwaiter;

    template <typename LazyType, typename IAlloc>
    friend struct detail::CollectAnyAwaiter;

    template <template <typename> typename LazyType, typename... Ts>
    friend struct detail::CollectAnyVariadicAwaiter;
};

// A RescheduleLazy is a Lazy with an executor. The executor of a RescheduleLazy
// wouldn't/shouldn't be nullptr. So we needn't check it.
//
// The user couldn't/shouldn't declare a coroutine function whose return type is
// RescheduleLazy. The user should get a RescheduleLazy by a call to
// `Lazy::via(Executor)` only.
//
// Different from Lazy, when a RescheduleLazy is co_awaited/started/syncAwaited,
// the RescheduleLazy wouldn't be executed immediately. The RescheduleLazy would
// submit a task to resume the corresponding Lazy task to the executor. Then the
// executor would execute the Lazy task later.
template <typename T = void>
class [[nodiscard]] RescheduleLazy {
public:
    using ValueType = typename Lazy<T>::ValueType;
    using Handle = typename Lazy<T>::Handle;

private:
    struct AwaiterBase : public detail::LazyAwaiterBase<T> {
        using Base = detail::LazyAwaiterBase<T>;
        AwaiterBase(Handle coro) : Base(coro) {}
        inline void await_suspend(CoroHandle<> continuation) {
            auto& pr = Base::_handle.promise();
            pr._continuation = continuation;
            // if co_await on RescheduleLazy, executor schedule performed
            std::function<void()> func = [h = Base::_handle]() mutable {
                h.resume();
            };
            pr._executor->schedule(func);
        }
    };
    struct ValueAwaiter : public AwaiterBase {
        ValueAwaiter(Handle coro) : AwaiterBase(coro) {}

        AS_INLINE T await_resume() { return AwaiterBase::awaitResume(); }
    };

    struct TryAwaiter : public AwaiterBase {
        TryAwaiter(Handle coro) : AwaiterBase(coro) {}
        AS_INLINE Try<T> await_resume() noexcept {
            return AwaiterBase::awaitResumeTry();
        };
    };

private:
    RescheduleLazy(Handle coro) : _coro(coro) {}

public:
    RescheduleLazy(const RescheduleLazy&) = delete;
    RescheduleLazy& operator=(const RescheduleLazy&) = delete;
    RescheduleLazy(RescheduleLazy&& other)
        : _coro(std::exchange(other._coro, nullptr)) {}

    Executor* getExecutor() { return _coro.promise()._executor; }

    auto operator co_await() noexcept {
        return ValueAwaiter(std::exchange(_coro, nullptr));
    }

    auto coAwaitTry() { return TryAwaiter(std::exchange(_coro, nullptr)); }

    template <typename F>
    void start(F&& callback) {
        auto launchCoro = [](RescheduleLazy lazy,
                             std::decay_t<F> cb) -> detail::DetachedCoroutine {
            cb(std::move(co_await lazy.coAwaitTry()));
        };
        [[maybe_unused]] auto detached =
            launchCoro(std::move(*this), std::forward<F>(callback));
    }

    void detach() {
        start([](auto&& t) {
            if (t.hasError()) {
                std::rethrow_exception(t.getException());
            }
        });
    }

private:
    Handle _coro;
    friend class Lazy<T>;

    template <typename LazyType, typename IAlloc, typename OAlloc, bool Para>
    friend struct detail::CollectAllAwaiter;

    template <typename LazyType, typename IAlloc>
    friend struct detail::CollectAnyAwaiter;

    template <template <typename> typename LazyType, typename... Ts>
    friend struct detail::CollectAnyVariadicAwaiter;
};

template <typename T>
inline Lazy<T> detail::LazyPromise<T>::get_return_object() noexcept {
    return Lazy<T>(Lazy<T>::Handle::from_promise(*this));
}

inline Lazy<void> detail::LazyPromise<void>::get_return_object() noexcept {
    return Lazy<void>(Lazy<void>::Handle::from_promise(*this));
}

}  // namespace coro
}  // namespace async_simple

#endif
