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
#ifndef ASYNC_SIMPLE_CORO_COLLECT_H
#define ASYNC_SIMPLE_CORO_COLLECT_H

#include <array>
#include <exception>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include "async_simple/Common.h"
#include "async_simple/Try.h"
#include "async_simple/Unit.h"
#include "async_simple/coro/CountEvent.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/experimental/coroutine.h"

namespace async_simple {
namespace coro {

// collectAll types
// auto [x, y] = co_await collectAll(IntLazy, FloatLazy);
// auto [x, y] = co_await collectAllPara(IntLazy, FloatLazy);
// std::vector<Try<int>> = co_await collectAll(std::vector<intLazy>);
// std::vector<Try<int>> = co_await collectAllPara(std::vector<intLazy>);
// std::vector<Try<int>> = co_await collectAllWindowed(maxConcurrency, yield,
// std::vector<intLazy>); std::vector<Try<int>> = co_await
// collectAllWindowedPara(maxConcurrency, yield, std::vector<intLazy>);

namespace detail {

template <typename T>
struct CollectAnyResult {
    CollectAnyResult() : _idx(static_cast<size_t>(-1)), _value() {}
    CollectAnyResult(size_t idx, std::add_rvalue_reference_t<T> value) requires(
        !std::is_void_v<T>)
        : _idx(idx), _value(std::move(value)) {}

    CollectAnyResult(const CollectAnyResult&) = delete;
    CollectAnyResult& operator=(const CollectAnyResult&) = delete;
    CollectAnyResult(CollectAnyResult&& other)
        : _idx(std::move(other._idx)), _value(std::move(other._value)) {
        other._idx = static_cast<size_t>(-1);
    }

    size_t _idx;
    Try<T> _value;

    size_t index() const { return _idx; }

    bool hasError() const { return _value.hasError(); }
    // Require hasError() == true. Otherwise it is UB to call
    // this method.
    std::exception_ptr getException() const { return _value.getException(); }

    // Require hasError() == false. Otherwise it is UB to call
    // value() method.
#if __cpp_explicit_this_parameter >= 202110L
    template <class Self>
    auto&& value(this Self&& self) {
        return std::forward<Self>(self)._value.value();
    }
#else
    const T& value() const& { return _value.value(); }
    T& value() & { return _value.value(); }
    T&& value() && { return std::move(_value).value(); }
    const T&& value() const&& { return std::move(_value).value(); }
#endif
};

template <typename LazyType, typename InAlloc, typename Callback = Unit>
struct CollectAnyAwaiter {
    using ValueType = typename LazyType::ValueType;
    using ResultType = CollectAnyResult<ValueType>;

    CollectAnyAwaiter(std::vector<LazyType, InAlloc>&& input)
        : _input(std::move(input)), _result(nullptr) {}

    CollectAnyAwaiter(std::vector<LazyType, InAlloc>&& input, Callback callback)
        : _input(std::move(input)),
          _result(nullptr),
          _callback(std::move(callback)) {}

    CollectAnyAwaiter(const CollectAnyAwaiter&) = delete;
    CollectAnyAwaiter& operator=(const CollectAnyAwaiter&) = delete;
    CollectAnyAwaiter(CollectAnyAwaiter&& other)
        : _input(std::move(other._input)),
          _result(std::move(other._result)),
          _callback(std::move(other._callback)) {}

    bool await_ready() const noexcept {
        return _input.empty() ||
               (_result && _result->_idx != static_cast<size_t>(-1));
    }

    void await_suspend(std::coroutine_handle<> continuation) {
        auto promise_type =
            std::coroutine_handle<LazyPromiseBase>::from_address(
                continuation.address())
                .promise();
        auto executor = promise_type._executor;
        // we should take care of input's life-time after resume.
        std::vector<LazyType, InAlloc> input(std::move(_input));
        // Make local copies to shared_ptr to avoid deleting objects too early
        // if any coroutine finishes before this function.
        auto result = std::make_shared<ResultType>();
        auto event = std::make_shared<detail::CountEvent>(input.size());
        auto callback = std::move(_callback);

        _result = result;
        for (size_t i = 0;
             i < input.size() && (result->_idx == static_cast<size_t>(-1));
             ++i) {
            if (!input[i]._coro.promise()._executor) {
                input[i]._coro.promise()._executor = executor;
            }

            if constexpr (std::is_same_v<Callback, Unit>) {
                (void)callback;
                input[i].start([i, size = input.size(), r = result,
                                c = continuation,
                                e = event](Try<ValueType>&& result) mutable {
                    assert(e != nullptr);
                    auto count = e->downCount();
                    if (count == size + 1) {
                        r->_idx = i;
                        r->_value = std::move(result);
                        c.resume();
                    }
                });
            } else {
                input[i].start([i, size = input.size(), r = result,
                                c = continuation, e = event,
                                callback](Try<ValueType>&& result) mutable {
                    assert(e != nullptr);
                    auto count = e->downCount();
                    if (count == size + 1) {
                        r->_idx = i;
                        (*callback)(i, std::move(result));
                        c.resume();
                    }
                });
            }
        }  // end for
    }
    auto await_resume() {
        if constexpr (std::is_same_v<Callback, Unit>) {
            assert(_result != nullptr);
            return std::move(*_result);
        } else {
            return _result->index();
        }
    }

    std::vector<LazyType, InAlloc> _input;
    std::shared_ptr<ResultType> _result;
    [[no_unique_address]] Callback _callback;
};

template <typename... Ts>
struct CollectAnyVariadicPairAwaiter {
    using InputType = std::tuple<Ts...>;

    CollectAnyVariadicPairAwaiter(Ts&&... inputs)
        : _input(std::move(inputs)...), _result(nullptr) {}

    CollectAnyVariadicPairAwaiter(InputType&& inputs)
        : _input(std::move(inputs)), _result(nullptr) {}

    CollectAnyVariadicPairAwaiter(const CollectAnyVariadicPairAwaiter&) =
        delete;
    CollectAnyVariadicPairAwaiter& operator=(
        const CollectAnyVariadicPairAwaiter&) = delete;
    CollectAnyVariadicPairAwaiter(CollectAnyVariadicPairAwaiter&& other)
        : _input(std::move(other._input)), _result(std::move(other._result)) {}

    bool await_ready() const noexcept {
        return _result && _result->has_value();
    }

    void await_suspend(std::coroutine_handle<> continuation) {
        auto promise_type =
            std::coroutine_handle<LazyPromiseBase>::from_address(
                continuation.address())
                .promise();
        auto executor = promise_type._executor;
        auto event =
            std::make_shared<detail::CountEvent>(std::tuple_size<InputType>());
        auto result = std::make_shared<std::optional<size_t>>();
        _result = result;

        auto input = std::move(_input);

        [&]<size_t... I>(std::index_sequence<I...>) {
            (
                [&](auto& lazy, auto& callback) {
                    if (result->has_value()) {
                        return;
                    }

                    if (!lazy._coro.promise()._executor) {
                        lazy._coro.promise()._executor = executor;
                    }

                    lazy.start([result, event, continuation,
                                callback](auto&& res) mutable {
                        auto count = event->downCount();
                        if (count == std::tuple_size<InputType>() + 1) {
                            callback(std::move(res));
                            *result = I;
                            continuation.resume();
                        }
                    });
                }(std::get<0>(std::get<I>(input)),
                  std::get<1>(std::get<I>(input))),
                ...);
        }
        (std::make_index_sequence<sizeof...(Ts)>());
    }

    auto await_resume() {
        assert(_result != nullptr);
        return std::move(_result->value());
    }

    std::tuple<Ts...> _input;
    std::shared_ptr<std::optional<size_t>> _result;
};

template <typename... Ts>
struct SimpleCollectAnyVariadicPairAwaiter {
    using InputType = std::tuple<Ts...>;

    InputType _inputs;

    SimpleCollectAnyVariadicPairAwaiter(Ts&&... inputs)
        : _inputs(std::move(inputs)...) {}

    auto coAwait(Executor* ex) {
        return CollectAnyVariadicPairAwaiter(std::move(_inputs));
    }
};

template <template <typename> typename LazyType, typename... Ts>
struct CollectAnyVariadicAwaiter {
    using ResultType = std::variant<Try<Ts>...>;
    using InputType = std::tuple<LazyType<Ts>...>;

    CollectAnyVariadicAwaiter(LazyType<Ts>&&... inputs)
        : _input(std::make_unique<InputType>(std::move(inputs)...)),
          _result(nullptr) {}

    CollectAnyVariadicAwaiter(InputType&& inputs)
        : _input(std::make_unique<InputType>(std::move(inputs))),
          _result(nullptr) {}

    CollectAnyVariadicAwaiter(const CollectAnyVariadicAwaiter&) = delete;
    CollectAnyVariadicAwaiter& operator=(const CollectAnyVariadicAwaiter&) =
        delete;
    CollectAnyVariadicAwaiter(CollectAnyVariadicAwaiter&& other)
        : _input(std::move(other._input)), _result(std::move(other._result)) {}

    bool await_ready() const noexcept {
        return _result && _result->has_value();
    }

    template <size_t... index>
    void await_suspend_impl(std::index_sequence<index...>,
                            std::coroutine_handle<> continuation) {
        auto promise_type =
            std::coroutine_handle<LazyPromiseBase>::from_address(
                continuation.address())
                .promise();
        auto executor = promise_type._executor;

        auto input = std::move(_input);
        // Make local copies to shared_ptr to avoid deleting objects too early
        // if any coroutine finishes before this function.
        auto result = std::make_shared<std::optional<ResultType>>();
        auto event =
            std::make_shared<detail::CountEvent>(std::tuple_size<InputType>());

        _result = result;

        (
            [&]() {
                if (result->has_value()) {
                    return;
                }
                if (!std::get<index>(*input)._coro.promise()._executor) {
                    std::get<index>(*input)._coro.promise()._executor =
                        executor;
                }
                std::get<index>(*input).start(
                    [r = result, c = continuation,
                     e = event](std::variant_alternative_t<index, ResultType>&&
                                    res) mutable {
                        assert(e != nullptr);
                        auto count = e->downCount();
                        if (count == std::tuple_size<InputType>() + 1) {
                            *r = ResultType{std::in_place_index_t<index>(),
                                            std::move(res)};
                            c.resume();
                        }
                    });
            }(),
            ...);
    }

    void await_suspend(std::coroutine_handle<> continuation) {
        await_suspend_impl(std::make_index_sequence<sizeof...(Ts)>{},
                           std::move(continuation));
    }

    auto await_resume() {
        assert(_result != nullptr);
        return std::move(_result->value());
    }

    std::unique_ptr<std::tuple<LazyType<Ts>...>> _input;
    std::shared_ptr<std::optional<ResultType>> _result;
};

template <typename T, typename InAlloc, typename Callback = Unit>
struct SimpleCollectAnyAwaitable {
    using ValueType = T;
    using LazyType = Lazy<T>;
    using VectorType = std::vector<LazyType, InAlloc>;

    VectorType _input;
    [[no_unique_address]] Callback _callback;

    SimpleCollectAnyAwaitable(std::vector<LazyType, InAlloc>&& input)
        : _input(std::move(input)) {}

    SimpleCollectAnyAwaitable(std::vector<LazyType, InAlloc>&& input,
                              Callback callback)
        : _input(std::move(input)), _callback(std::move(callback)) {}

    auto coAwait(Executor* ex) {
        if constexpr (std::is_same_v<Callback, Unit>) {
            return CollectAnyAwaiter<LazyType, InAlloc>(std::move(_input));
        } else {
            return CollectAnyAwaiter<LazyType, InAlloc, Callback>(
                std::move(_input), std::move(_callback));
        }
    }
};

template <template <typename> typename LazyType, typename... Ts>
struct SimpleCollectAnyVariadicAwaiter {
    using InputType = std::tuple<LazyType<Ts>...>;

    InputType _inputs;

    SimpleCollectAnyVariadicAwaiter(LazyType<Ts>&&... inputs)
        : _inputs(std::move(inputs)...) {}

    auto coAwait(Executor* ex) {
        return CollectAnyVariadicAwaiter(std::move(_inputs));
    }
};

template <class Container, typename OAlloc, bool Para = false>
struct CollectAllAwaiter {
    using ValueType = typename Container::value_type::ValueType;

    CollectAllAwaiter(Container&& input, OAlloc outAlloc)
        : _input(std::move(input)), _output(outAlloc), _event(_input.size()) {
        _output.resize(_input.size());
    }
    CollectAllAwaiter(CollectAllAwaiter&& other) = default;

    CollectAllAwaiter(const CollectAllAwaiter&) = delete;
    CollectAllAwaiter& operator=(const CollectAllAwaiter&) = delete;

    inline bool await_ready() const noexcept { return _input.empty(); }
    inline void await_suspend(std::coroutine_handle<> continuation) {
        auto promise_type =
            std::coroutine_handle<LazyPromiseBase>::from_address(
                continuation.address())
                .promise();
        auto executor = promise_type._executor;
        for (size_t i = 0; i < _input.size(); ++i) {
            auto& exec = _input[i]._coro.promise()._executor;
            if (exec == nullptr) {
                exec = executor;
            }
            auto&& func = [this, i]() {
                _input[i].start([this, i](Try<ValueType>&& result) {
                    _output[i] = std::move(result);
                    auto awaitingCoro = _event.down();
                    if (awaitingCoro) {
                        awaitingCoro.resume();
                    }
                });
            };
            if (Para == true && _input.size() > 1) {
                if (exec != nullptr)
                    AS_LIKELY {
                        exec->schedule(func);
                        continue;
                    }
            }
            func();
        }
        _event.setAwaitingCoro(continuation);
        auto awaitingCoro = _event.down();
        if (awaitingCoro) {
            awaitingCoro.resume();
        }
    }
    inline auto await_resume() { return std::move(_output); }

    Container _input;
    std::vector<Try<ValueType>, OAlloc> _output;
    detail::CountEvent _event;
};  // CollectAllAwaiter

template <class Container, typename OAlloc, bool Para = false>
struct SimpleCollectAllAwaitable {
    Container _input;
    OAlloc _out_alloc;

    SimpleCollectAllAwaitable(Container&& input, OAlloc out_alloc)
        : _input(std::move(input)), _out_alloc(out_alloc) {}

    auto coAwait(Executor* ex) {
        return CollectAllAwaiter<Container, OAlloc, Para>(std::move(_input),
                                                          _out_alloc);
    }
};  // SimpleCollectAllAwaitable

}  // namespace detail

namespace detail {

template <typename T>
struct is_lazy : std::false_type {};

template <typename T>
struct is_lazy<Lazy<T>> : std::true_type {};

template <bool Para, class Container,
          typename T = typename Container::value_type::ValueType,
          typename OAlloc = std::allocator<Try<T>>>
inline auto collectAllImpl(Container input, OAlloc out_alloc = OAlloc()) {
    using LazyType = typename Container::value_type;
    using AT = std::conditional_t<
        is_lazy<LazyType>::value,
        detail::SimpleCollectAllAwaitable<Container, OAlloc, Para>,
        detail::CollectAllAwaiter<Container, OAlloc, Para>>;
    return AT(std::move(input), out_alloc);
}

template <bool Para, class Container,
          typename T = typename Container::value_type::ValueType,
          typename OAlloc = std::allocator<Try<T>>>
inline auto collectAllWindowedImpl(size_t maxConcurrency,
                                   bool yield /*yield between two batchs*/,
                                   Container input, OAlloc out_alloc = OAlloc())
    -> Lazy<std::vector<Try<T>, OAlloc>> {
    using LazyType = typename Container::value_type;
    using AT = std::conditional_t<
        is_lazy<LazyType>::value,
        detail::SimpleCollectAllAwaitable<Container, OAlloc, Para>,
        detail::CollectAllAwaiter<Container, OAlloc, Para>>;
    std::vector<Try<T>, OAlloc> output(out_alloc);
    output.reserve(input.size());
    size_t input_size = input.size();
    // maxConcurrency == 0;
    // input_size <= maxConcurrency size;
    // act just like CollectAll.
    if (maxConcurrency == 0 || input_size <= maxConcurrency) {
        co_return co_await AT(std::move(input), out_alloc);
    }
    size_t start = 0;
    while (start < input_size) {
        size_t end = (std::min)(
            input_size,
            start + maxConcurrency);  // Avoid to conflict with Windows macros.
        std::vector<LazyType> tmp_group(
            std::make_move_iterator(input.begin() + start),
            std::make_move_iterator(input.begin() + end));
        start = end;
        for (auto& t : co_await AT(std::move(tmp_group), out_alloc)) {
            output.push_back(std::move(t));
        }
        if (yield) {
            co_await Yield{};
        }
    }
    co_return std::move(output);
}

// variadic collectAll

template <bool Para, template <typename> typename LazyType, typename... Ts>
struct CollectAllVariadicAwaiter {
    using ResultType = std::tuple<Try<Ts>...>;
    using InputType = std::tuple<LazyType<Ts>...>;

    CollectAllVariadicAwaiter(LazyType<Ts>&&... inputs)
        : _inputs(std::move(inputs)...), _event(sizeof...(Ts)) {}
    CollectAllVariadicAwaiter(InputType&& inputs)
        : _inputs(std::move(inputs)), _event(sizeof...(Ts)) {}

    CollectAllVariadicAwaiter(const CollectAllVariadicAwaiter&) = delete;
    CollectAllVariadicAwaiter& operator=(const CollectAllVariadicAwaiter&) =
        delete;
    CollectAllVariadicAwaiter(CollectAllVariadicAwaiter&&) = default;

    bool await_ready() const noexcept { return false; }

    template <size_t... index>
    void await_suspend_impl(std::index_sequence<index...>,
                            std::coroutine_handle<> continuation) {
        auto promise_type =
            std::coroutine_handle<LazyPromiseBase>::from_address(
                continuation.address())
                .promise();
        auto executor = promise_type._executor;

        _event.setAwaitingCoro(continuation);

        // fold expression
        (
            [executor, this](auto& lazy, auto& result) {
                auto& exec = lazy._coro.promise()._executor;
                if (exec == nullptr) {
                    exec = executor;
                }
                auto func = [&]() {
                    lazy.start([&](auto&& res) {
                        result = std::move(res);
                        if (auto awaitingCoro = _event.down(); awaitingCoro) {
                            awaitingCoro.resume();
                        }
                    });
                };

                if constexpr (Para == true && sizeof...(Ts) > 1) {
                    if (exec != nullptr)
                        AS_LIKELY { exec->schedule(std::move(func)); }
                    else
                        AS_UNLIKELY { func(); }
                } else {
                    func();
                }
            }(std::get<index>(_inputs), std::get<index>(_results)),
            ...);

        if (auto awaitingCoro = _event.down(); awaitingCoro) {
            awaitingCoro.resume();
        }
    }

    void await_suspend(std::coroutine_handle<> continuation) {
        await_suspend_impl(std::make_index_sequence<sizeof...(Ts)>{},
                           std::move(continuation));
    }

    auto await_resume() { return std::move(_results); }

    InputType _inputs;
    ResultType _results;
    detail::CountEvent _event;
};

template <bool Para, template <typename> typename LazyType, typename... Ts>
struct SimpleCollectAllVariadicAwaiter {
    using InputType = std::tuple<LazyType<Ts>...>;

    SimpleCollectAllVariadicAwaiter(LazyType<Ts>&&... inputs)
        : _input(std::move(inputs)...) {}

    auto coAwait(Executor* ex) {
        return CollectAllVariadicAwaiter<Para, LazyType, Ts...>(
            std::move(_input));
    }

    InputType _input;
};

template <bool Para, template <typename> typename LazyType, typename... Ts>
inline auto collectAllVariadicImpl(LazyType<Ts>&&... awaitables) {
    static_assert(sizeof...(Ts) > 0);
    using AT = std::conditional_t<
        is_lazy<LazyType<void>>::value && !Para,
        SimpleCollectAllVariadicAwaiter<Para, LazyType, Ts...>,
        CollectAllVariadicAwaiter<Para, LazyType, Ts...>>;
    return AT(std::move(awaitables)...);
}

// collectAny
template <typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>,
          typename Callback = Unit>
inline auto collectAnyImpl(std::vector<LazyType<T>, IAlloc> input,
                           Callback callback = {}) {
    using AT = std::conditional_t<
        std::is_same_v<LazyType<T>, Lazy<T>>,
        detail::SimpleCollectAnyAwaitable<T, IAlloc, Callback>,
        detail::CollectAnyAwaiter<LazyType<T>, IAlloc, Callback>>;
    return AT(std::move(input), std::move(callback));
}

// collectAnyVariadic
template <template <typename> typename LazyType, typename... Ts>
inline auto CollectAnyVariadicImpl(LazyType<Ts>&&... inputs) {
    using AT =
        std::conditional_t<is_lazy<LazyType<void>>::value,
                           SimpleCollectAnyVariadicAwaiter<LazyType, Ts...>,
                           CollectAnyVariadicAwaiter<LazyType, Ts...>>;
    return AT(std::move(inputs)...);
}

// collectAnyVariadicPair
template <typename T, typename... Ts>
inline auto CollectAnyVariadicPairImpl(T&& input, Ts&&... inputs) {
    using U = std::tuple_element_t<0, std::remove_cvref_t<T>>;
    using AT = std::conditional_t<is_lazy<U>::value,
                                  SimpleCollectAnyVariadicPairAwaiter<T, Ts...>,
                                  CollectAnyVariadicPairAwaiter<T, Ts...>>;
    return AT(std::move(input), std::move(inputs)...);
}
}  // namespace detail

template <typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>>
inline auto collectAny(std::vector<LazyType<T>, IAlloc>&& input) {
    return detail::collectAnyImpl(std::move(input));
}

template <typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>, typename Callback>
inline auto collectAny(std::vector<LazyType<T>, IAlloc>&& input,
                       Callback callback) {
    auto cb = std::make_shared<Callback>(std::move(callback));
    return detail::collectAnyImpl(std::move(input), std::move(cb));
}

template <template <typename> typename LazyType, typename... Ts>
inline auto collectAny(LazyType<Ts>... awaitables) {
    static_assert(sizeof...(Ts), "collectAny need at least one param!");
    return detail::CollectAnyVariadicImpl(std::move(awaitables)...);
}

// collectAny with std::pair<Lazy, CallbackFunction>
template <typename... Ts>
inline auto collectAny(Ts&&... inputs) {
    static_assert(sizeof...(Ts), "collectAny need at least one param!");
    return detail::CollectAnyVariadicPairImpl(std::move(inputs)...);
}

// The collectAll() function can be used to co_await on a vector of LazyType
// tasks in **one thread**,and producing a vector of Try values containing each
// of the results.
template <typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>,
          typename OAlloc = std::allocator<Try<T>>>
inline auto collectAll(std::vector<LazyType<T>, IAlloc>&& input,
                       OAlloc out_alloc = OAlloc()) {
    return detail::collectAllImpl<false>(std::move(input), out_alloc);
}

// Like the collectAll() function above, The collectAllPara() function can be
// used to concurrently co_await on a vector LazyType tasks in executor,and
// producing a vector of Try values containing each of the results.
template <typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>,
          typename OAlloc = std::allocator<Try<T>>>
inline auto collectAllPara(std::vector<LazyType<T>, IAlloc>&& input,
                           OAlloc out_alloc = OAlloc()) {
    return detail::collectAllImpl<true>(std::move(input), out_alloc);
}

// This collectAll function can be used to co_await on some different kinds of
// LazyType tasks in one thread,and producing a tuple of Try values containing
// each of the results.
template <template <typename> typename LazyType, typename... Ts>
// The temporary object's life-time which binding to reference(inputs) won't
// be extended to next time of coroutine resume. Just Copy inputs to avoid
// crash.
inline auto collectAll(LazyType<Ts>... inputs) {
    static_assert(sizeof...(Ts), "collectAll need at least one param!");
    return detail::collectAllVariadicImpl<false>(std::move(inputs)...);
}

// Like the collectAll() function above, This collectAllPara() function can be
// used to concurrently co_await on some different kinds of LazyType tasks in
// executor,and producing a tuple of Try values containing each of the results.
template <template <typename> typename LazyType, typename... Ts>
inline auto collectAllPara(LazyType<Ts>... inputs) {
    static_assert(sizeof...(Ts), "collectAllPara need at least one param!");
    return detail::collectAllVariadicImpl<true>(std::move(inputs)...);
}

// Await each of the input LazyType tasks in the vector, allowing at most
// 'maxConcurrency' of these input tasks to be awaited in one thread. yield is
// true: yield collectAllWindowedPara from thread when a 'maxConcurrency' of
// tasks is done.
template <typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>,
          typename OAlloc = std::allocator<Try<T>>>
inline auto collectAllWindowed(size_t maxConcurrency,
                               bool yield /*yield between two batchs*/,
                               std::vector<LazyType<T>, IAlloc>&& input,
                               OAlloc out_alloc = OAlloc()) {
    return detail::collectAllWindowedImpl<true>(maxConcurrency, yield,
                                                std::move(input), out_alloc);
}

// Await each of the input LazyType tasks in the vector, allowing at most
// 'maxConcurrency' of these input tasks to be concurrently awaited at any one
// point in time.
// yield is true: yield collectAllWindowedPara from thread when a
// 'maxConcurrency' of tasks is done.
template <typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>,
          typename OAlloc = std::allocator<Try<T>>>
inline auto collectAllWindowedPara(size_t maxConcurrency,
                                   bool yield /*yield between two batchs*/,
                                   std::vector<LazyType<T>, IAlloc>&& input,
                                   OAlloc out_alloc = OAlloc()) {
    return detail::collectAllWindowedImpl<false>(maxConcurrency, yield,
                                                 std::move(input), out_alloc);
}

}  // namespace coro
}  // namespace async_simple

#endif
