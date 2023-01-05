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

#include <cassert>
#include <concepts>
#include <cstddef>
#include <exception>
#include <ranges>
#include <type_traits>
#include <utility>

#include "async_simple/Common.h"
#include "async_simple/coro/PromiseAllocator.h"
#include "async_simple/experimental/coroutine.h"

namespace async_simple::ranges {

// For internal use only, for compatibility with lower version standard
// libraries
namespace internal {

// clang-format off
template <typename T>
concept range = requires(T& t) {
    std::ranges::begin(t);
    std::ranges::end(t);
};

template <class T>
using iterator_t = decltype(std::ranges::begin(std::declval<T&>()));

template <range R>
using sentinel_t = decltype(std::ranges::end(std::declval<R&>()));

template <range R>
using range_value_t = std::iter_value_t<iterator_t<R>>;

template <range R>
using range_reference_t = std::iter_reference_t<iterator_t<R>>;

template <class T>
concept input_range = range<T> && std::input_iterator<iterator_t<T>>;

}  // namespace internal

// clang-format on

#ifdef _MSC_VER
#define EMPTY_BASES __declspec(empty_bases)
#ifdef __clang__
#define NO_UNIQUE_ADDRESS
#else
#define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#endif
#else
#define EMPTY_BASES
#define NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

template <class R, class Alloc = std::allocator<std::byte>>
struct elements_of {
    NO_UNIQUE_ADDRESS R range;
    NO_UNIQUE_ADDRESS Alloc allocator{};
};

template <class R, class Alloc = std::allocator<std::byte>>
elements_of(R&&, Alloc = {}) -> elements_of<R&&, Alloc>;

}  // namespace async_simple::ranges

namespace async_simple::coro::detail {

template <class yield>
struct gen_promise_base;

}  // namespace async_simple::coro::detail

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

    template <class Yield>
    friend struct detail::gen_promise_base;
};

namespace detail {

template <class yielded>
struct gen_promise_base {
protected:
    struct NestInfo {
        std::exception_ptr except_;
        std::coroutine_handle<gen_promise_base> parent_;
        std::coroutine_handle<gen_promise_base> root_;
    };

    template <class R2, class V2, class Alloc2>
    struct NestedAwaiter {
        NestInfo nested_;
        Generator<R2, V2, Alloc2> gen_;

        explicit NestedAwaiter(Generator<R2, V2, Alloc2>&& gen) noexcept
            : gen_(std::move(gen)) {}
        bool await_ready() noexcept { return !gen_._coro; }

        template <class Promise>
        std::coroutine_handle<> await_suspend(
            std::coroutine_handle<Promise> handle) noexcept {
            auto target = std::coroutine_handle<gen_promise_base>::from_address(
                gen_._coro.address());
            nested_.parent_ =
                std::coroutine_handle<gen_promise_base>::from_address(
                    handle.address());
            auto& current = handle.promise();
            if (current.info_) {
                nested_.root_ = current.info_->root_;
            } else {
                nested_.root_ = nested_.parent_;
            }
            nested_.root_.promise().top_ = target;
            target.promise().info_ = std::addressof(nested_);
            return target;
        }

        void await_resume() {
            if (nested_.except_) {
                std::rethrow_exception(std::move(nested_.except_));
            }
        }
    };

    struct ElementAwaiter {
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

    struct FinalAwaiter {
        bool await_ready() const noexcept { return false; }

        template <class Promise>
        std::coroutine_handle<> await_suspend(
            std::coroutine_handle<Promise> handle) noexcept {
            auto& current = handle.promise();

            if (!current.info_) {
                return std::noop_coroutine();
            }

            auto previous = current.info_->parent_;
            current.info_->root_.promise().top_ = previous;
            current.info_ = nullptr;
            return previous;
        }

        void await_resume() noexcept {}
    };

    template <class Ref, class V, class Alloc>
    friend class async_simple::coro::Generator;

    std::add_pointer_t<yielded> value_ = nullptr;
    std::coroutine_handle<gen_promise_base> top_ =
        std::coroutine_handle<gen_promise_base>::from_promise(*this);
    NestInfo* info_ = nullptr;
};

}  // namespace detail

template <class Ref, class V, class Allocator>
class Generator<Ref, V, Allocator>::promise_type
    : public detail::gen_promise_base<yielded>,
      public detail::PromiseAllocator<Allocator> {
public:
    using Base = detail::gen_promise_base<yielded>;

    std::suspend_always initial_suspend() noexcept { return {}; }
    // When info is `nullptr`, equivalent to std::suspend_always
    auto final_suspend() noexcept { return typename Base::FinalAwaiter{}; }

    std::suspend_always yield_value(yielded val) noexcept {
        this->value_ = std::addressof(val);
        return {};
    }

    // clang-format off
    auto yield_value(const std::remove_reference_t<yielded>& lval) requires
        std::is_rvalue_reference_v<yielded> && std::constructible_from<
        std::remove_cvref_t<yielded>, const std::remove_reference_t<yielded>& > 
    { return typename Base::ElementAwaiter{lval}; }
    // clang-format on

    // clang-format off
    template <class R2, class V2, class Alloc2, class Unused>
    requires std::same_as<typename Generator<R2, V2, Alloc2>::yielded, yielded>
    auto yield_value(
        ranges::elements_of<Generator<R2, V2, Alloc2>&&, Unused> g) noexcept {
        // clang-format on
        return typename Base::template NestedAwaiter<R2, V2, Alloc2>{
            std::move(g.range)};
    }

    // clang-format off
    template <class R, class Alloc>
    requires std::convertible_to<ranges::internal::range_reference_t<R>, yielded>
    auto yield_value(ranges::elements_of<R, Alloc> r) noexcept {
        // clang-format on
        auto nested = [](std::allocator_arg_t, Alloc,
                         ranges::internal::iterator_t<R> i,
                         ranges::internal::sentinel_t<R> s)
            -> Generator<yielded, ranges::internal::range_value_t<R>, Alloc> {
            for (; i != s; ++i) {
                co_yield static_cast<yielded>(*i);
            }
        };
        return yield_value(ranges::elements_of{
            nested(std::allocator_arg, r.allocator, std::ranges::begin(r.range),
                   std::ranges::end(r.range))});
    }

    void return_void() noexcept {}
    void await_transform() = delete;
    void unhandled_exception() {
        if (this->info_) {
            this->info_->except_ = std::current_exception();
        } else {
            throw;
        }
    }

    Generator get_return_object() noexcept {
        return Generator(Generator::Handle::from_promise(*this));
    }
};

template <class Ref, class V, class Allocator>
class Generator<Ref, V, Allocator>::iterator {
public:
    using value_type = value;
    using difference_type = std::ptrdiff_t;

    explicit iterator(Handle coro) noexcept : _coro(coro) {}
    ~iterator() {
        if (_coro) {
            if (!_coro.done()) {
                // TODO: error log
            }
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
        assert(_coro.promise().top_.promise().value_ &&
               "value pointer is nullptr");
        return static_cast<yielded>(*_coro.promise().top_.promise().value_);
    }

    iterator& operator++() {
        logicAssert(this->operator bool(),
                    "Can't increment generator end iterator");
        _coro.promise().top_.resume();
        return *this;
    }

    void operator++(int) { ++(*this); }

private:
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
    logicAssert(!!_coro, "Can't call begin on moved-from generator");
    _coro.resume();
    return iterator(std::exchange(_coro, nullptr));
}

}  // namespace async_simple::coro

template <class Ref, class V, class Allocator>
inline constexpr bool
    std::ranges::enable_view<async_simple::coro::Generator<Ref, V, Allocator>> =
        true;

#endif
