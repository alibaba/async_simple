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

#include <async_simple/Common.h>
#include <async_simple/Try.h>
#include <async_simple/coro/Event.h>
#include <async_simple/coro/Lazy.h>
#include <async_simple/experimental/coroutine.h>
#include <exception>
#include <vector>

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
    CollectAnyResult(size_t idx, T&& value)
        : _idx(idx), _value(std::move(value)) {}

    CollectAnyResult(const CollectAnyResult&) = delete;
    CollectAnyResult& operator=(const CollectAnyResult&) = delete;
    CollectAnyResult(CollectAnyResult&& other)
        : _idx(std::move(other._idx)), _value(std::move(other._value)) {
        other._idx = static_cast<size_t>(-1);
    }

    size_t _idx;
    Try<T> _value;
};

template <>
struct CollectAnyResult<void> {
    CollectAnyResult() : _idx(static_cast<size_t>(-1)) {}
    size_t _idx;
    Try<void> _value;
};

template <typename LazyType, typename InAlloc>
struct CollectAnyAwaiter {
    using ValueType = typename LazyType::ValueType;
    using ResultType = CollectAnyResult<ValueType>;

    CollectAnyAwaiter(std::vector<LazyType, InAlloc>&& input)
        : _input(std::move(input)), _result(nullptr) {}

    CollectAnyAwaiter(const CollectAnyAwaiter&) = delete;
    CollectAnyAwaiter& operator=(const CollectAnyAwaiter&) = delete;
    CollectAnyAwaiter(CollectAnyAwaiter&& other)
        : _input(std::move(other._input)), _result(std::move(other._result)) {}

    bool await_ready() const noexcept {
        return _input.empty() ||
               (_result && _result->_idx != static_cast<size_t>(-1));
    }

    void await_suspend(STD_CORO::coroutine_handle<> continuation) {
        auto promise_type =
            static_cast<STD_CORO::coroutine_handle<LazyPromiseBase>*>(
                &continuation)
                ->promise();
        auto executor = promise_type._executor;
        // Make local copies to shared_ptr to avoid deleting objects too early
        // if any coroutine finishes before this function.
        std::vector<LazyType, InAlloc> input(std::move(_input));
        auto result = std::make_shared<ResultType>();
        auto event = std::make_shared<detail::CountEvent>(input.size());

        _result = result;
        for (size_t i = 0;
             i < input.size() && (result->_idx == static_cast<size_t>(-1));
             ++i) {
            if (!input[i]._coro.promise()._executor) {
                input[i]._coro.promise()._executor = executor;
            }

            input[i].start([i = i, size = input.size(), r = result,
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
        }  // end for
    }
    auto await_resume() {
        assert(_result != nullptr);
        return std::move(*_result);
    }

    std::vector<LazyType, InAlloc> _input;
    std::shared_ptr<ResultType> _result;
};

template <typename T, typename InAlloc>
struct SimpleCollectAnyAwaitable {
    using ValueType = T;
    using LazyType = Lazy<T>;
    using VectorType = std::vector<LazyType, InAlloc>;

    VectorType _input;

    SimpleCollectAnyAwaitable(std::vector<LazyType, InAlloc>&& input)
        : _input(std::move(input)) {}

    auto coAwait(Executor* ex) {
        return CollectAnyAwaiter<LazyType, InAlloc>(std::move(_input));
    }
};

template <typename LazyType, typename IAlloc, typename OAlloc,
          bool Para = false>
struct CollectAllAwaiter {
    using ValueType = typename LazyType::ValueType;

    CollectAllAwaiter(std::vector<LazyType, IAlloc>&& input, OAlloc outAlloc)
        : _input(std::move(input)), _output(outAlloc), _event(_input.size()) {
        _output.resize(_input.size());
    }
    CollectAllAwaiter(CollectAllAwaiter&& other)
        : _input(std::move(other._input)),
          _output(std::move(other._output)),
          _event(std::move(other._event)) {}

    CollectAllAwaiter(const CollectAllAwaiter&) = delete;
    CollectAllAwaiter& operator=(const CollectAllAwaiter&) = delete;

    inline bool await_ready() const noexcept { return _input.empty(); }
    inline void await_suspend(STD_CORO::coroutine_handle<> continuation) {
        auto promise_type =
            static_cast<STD_CORO::coroutine_handle<LazyPromiseBase>*>(
                &continuation)
                ->promise();
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
                if (FL_LIKELY(exec != nullptr)) {
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

    std::vector<LazyType, IAlloc> _input;
    std::vector<Try<ValueType>, OAlloc> _output;
    detail::CountEvent _event;
};  // CollectAllAwaiter

template <typename T, typename IAlloc, typename OAlloc, bool Para = false>
struct SimpleCollectAllAwaitable {
    using ValueType = T;
    using LazyType = Lazy<T>;
    using VectorType = std::vector<LazyType, IAlloc>;

    VectorType _input;
    OAlloc _out_alloc;

    SimpleCollectAllAwaitable(VectorType&& input, OAlloc out_alloc)
        : _input(std::move(input)), _out_alloc(out_alloc) {}

    auto coAwait(Executor* ex) {
        return CollectAllAwaiter<LazyType, IAlloc, OAlloc, Para>(
            std::move(_input), _out_alloc);
    }
};  // SimpleCollectAllAwaitable

}  // namespace detail

// collectAny
template <typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>>
inline auto collectAny(std::vector<LazyType<T>, IAlloc>&& input)
    -> Lazy<detail::CollectAnyResult<T>> {
    using AT =
        std::conditional_t<std::is_same_v<LazyType<T>, Lazy<T>>,
                           detail::SimpleCollectAnyAwaitable<T, IAlloc>,
                           detail::CollectAnyAwaiter<LazyType<T>, IAlloc>>;
    co_return co_await AT(std::move(input));
}

namespace detail {

template <bool Para, typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>,
          typename OAlloc = std::allocator<Try<T>>>
inline auto collectAllImpl(std::vector<LazyType<T>, IAlloc>&& input,
                           OAlloc out_alloc = OAlloc())
    -> Lazy<std::vector<Try<T>, OAlloc>> {
    using AT = std::conditional_t<
        std::is_same_v<LazyType<T>, Lazy<T>>,
        detail::SimpleCollectAllAwaitable<T, IAlloc, OAlloc, Para>,
        detail::CollectAllAwaiter<LazyType<T>, IAlloc, OAlloc, Para>>;
    co_return co_await AT(std::move(input), out_alloc);
}

template <bool Para, typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>,
          typename OAlloc = std::allocator<Try<T>>>
inline auto collectAllWindowedImpl(size_t maxConcurrency,
                                   bool yield /*yield between two batchs*/,
                                   std::vector<LazyType<T>, IAlloc>&& input,
                                   OAlloc out_alloc = OAlloc())
    -> Lazy<std::vector<Try<T>, OAlloc>> {
    using AT = std::conditional_t<
        std::is_same_v<LazyType<T>, Lazy<T>>,
        detail::SimpleCollectAllAwaitable<T, IAlloc, OAlloc, Para>,
        detail::CollectAllAwaiter<LazyType<T>, IAlloc, OAlloc, Para>>;
    std::vector<Try<T>, OAlloc> output(out_alloc);
    size_t input_size = input.size();
    // maxConcurrency == 0;
    // input_size <= maxConcurrency size;
    // act just like CollectAll.
    if (maxConcurrency == 0 || input_size <= maxConcurrency) {
        co_return co_await AT(std::move(input), out_alloc);
    }
    size_t start = 0;
    while (start < input_size) {
        size_t end = std::min(input_size, start + maxConcurrency);
        std::vector<LazyType<T>, IAlloc> tmp_group(input.get_allocator());
        for (; start < end; ++start) {
            tmp_group.push_back(std::move(input[start]));
        }
        auto tmp_output = co_await AT(std::move(tmp_group), out_alloc);
        for (auto& t : tmp_output) {
            output.push_back(std::move(t));
        }
        if (yield) {
            co_await Yield();
        }
    }
    co_return std::move(output);
}

// variadic collectAll

template <template <typename> typename LazyType, typename Ts>
Lazy<void> makeWraperTask(LazyType<Ts>&& awaitable, Try<Ts>& result) {
    try {
        if constexpr (std::is_void_v<Ts>) {
            co_await awaitable;
        } else {
            result = co_await awaitable;
        }
    } catch (...) {
        result.setException(std::current_exception());
    }
}

template <bool Para, template <typename> typename LazyType, typename... Ts,
          size_t... Indices>
inline auto collectAllVariadicImpl(std::index_sequence<Indices...>,
                                   LazyType<Ts>&&... awaitables)
    -> Lazy<std::tuple<Try<Ts>...>> {
    static_assert(sizeof...(Ts) > 0);

    std::tuple<Try<Ts>...> results;
    std::vector<Lazy<void>> wraper_tasks;

    // make wraper task
    (void)std::initializer_list<bool>{
        (wraper_tasks.push_back(std::move(makeWraperTask(
             std::move(awaitables), std::get<Indices>(results)))),
         true)...,
    };

    co_await collectAllImpl<Para>(std::move(wraper_tasks));
    co_return std::move(results);
}
}  // namespace detail

// The collectAll() function can be used to co_await on a vector of LazyType
// tasks in **one thread**,and producing a vector of Try values containing each
// of the results.
template <typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>,
          typename OAlloc = std::allocator<Try<T>>>
inline auto collectAll(std::vector<LazyType<T>, IAlloc>&& input,
                       OAlloc out_alloc = OAlloc())
    -> Lazy<std::vector<Try<T>, OAlloc>> {
    co_return co_await detail::collectAllImpl<false>(std::move(input),
                                                     out_alloc);
}

// Like the collectAll() function above, The collectAllPara() function can be
// used to concurrently co_await on a vector LazyType tasks in executor,and
// producing a vector of Try values containing each of the results.
template <typename T, template <typename> typename LazyType,
          typename IAlloc = std::allocator<LazyType<T>>,
          typename OAlloc = std::allocator<Try<T>>>
inline auto collectAllPara(std::vector<LazyType<T>, IAlloc>&& input,
                           OAlloc out_alloc = OAlloc())
    -> Lazy<std::vector<Try<T>, OAlloc>> {
    co_return co_await detail::collectAllImpl<true>(std::move(input),
                                                    out_alloc);
}

// This collectAll function can be used to co_await on some different kinds of
// LazyType tasks in one thread,and producing a tuple of Try values containing
// each of the results.
template <template <typename> typename LazyType, typename... Ts>
inline auto collectAll(LazyType<Ts>&&... inputs)
    -> Lazy<std::tuple<Try<Ts>...>> {
    if constexpr (0 == sizeof...(Ts)) {
        co_return std::tuple<>{};
    } else {
        co_return co_await detail::collectAllVariadicImpl<false>(
            std::make_index_sequence<sizeof...(Ts)>{}, std::move(inputs)...);
    }
}

// Like the collectAll() function above, This collectAllPara() function can be
// used to concurrently co_await on some different kinds of LazyType tasks in
// executor,and producing a tuple of Try values containing each of the results.
template <template <typename> typename LazyType, typename... Ts>
inline auto collectAllPara(LazyType<Ts>&&... inputs)
    -> Lazy<std::tuple<Try<Ts>...>> {
    if constexpr (0 == sizeof...(Ts)) {
        co_return std::tuple<>{};
    } else {
        co_return co_await detail::collectAllVariadicImpl<true>(
            std::make_index_sequence<sizeof...(Ts)>{}, std::move(inputs)...);
    }
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
                               OAlloc out_alloc = OAlloc())
    -> Lazy<std::vector<Try<T>, OAlloc>> {
    co_return co_await detail::collectAllWindowedImpl<true>(
        maxConcurrency, yield, std::move(input), out_alloc);
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
                                   OAlloc out_alloc = OAlloc())
    -> Lazy<std::vector<Try<T>, OAlloc>> {
    co_return co_await detail::collectAllWindowedImpl<false>(
        maxConcurrency, yield, std::move(input), out_alloc);
}

}  // namespace coro
}  // namespace async_simple

#endif
