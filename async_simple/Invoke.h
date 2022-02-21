/*
 * Copyright 2017-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ASYNC_SIMPLE_INVOKE_H
#define ASYNC_SIMPLE_INVOKE_H

#include <async_simple/Common.h>
#include <exception>
#include <functional>
#include <type_traits>

// FIXME: __cpp_lib_is_invocable in my clang version is broken
#define USE_STD_LIBRARY 0

namespace async_simple {

/**
 *  include or backport:
 *  * std::invoke
 *  * std::invoke_result
 *  * std::invoke_result_t
 *  * std::is_invocable
 *  * std::is_invocable_r
 *  * std::is_nothrow_invocable
 */

#if __cpp_lib_is_invocable >= 201703 || USE_STD_LIBRARY

using std::invoke;

#else

//  mimic: std::invoke, C++17
template <typename F, typename... Args>
constexpr auto invoke(F&& f, Args&&... args) noexcept(
    noexcept(static_cast<F&&>(f)(static_cast<Args&&>(args)...)))
    -> decltype(static_cast<F&&>(f)(static_cast<Args&&>(args)...)) {
    return static_cast<F&&>(f)(static_cast<Args&&>(args)...);
}

template <typename M, typename C, typename... Args>
constexpr auto invoke(M(C::*d), Args&&... args)
    -> decltype(std::mem_fn(d)(static_cast<Args&&>(args)...)) {
    return std::mem_fn(d)(static_cast<Args&&>(args)...);
}

#endif

#if __cpp_lib_is_invocable >= 201703 || USE_STD_LIBRARY

/* using override */ using std::invoke_result;
/* using override */ using std::invoke_result_t;

#else

namespace invoke_detail {

#if __cpp_lib_bool_constant

using std::bool_constant;

#else

//  mimic: std::bool_constant, C++17
template <bool B>
using bool_constant = std::integral_constant<bool, B>;

#endif

// Until CWG 1558 (a C++14 defect), unused parameters in alias templates were
// not guaranteed to ensure SFINAE and could be ignored, so earlier compilers
// require a more complex definition of void_t, see
// https://en.cppreference.com/w/cpp/types/void_t

#if __cpp_lib_is_invocable >= 201703 || USE_STD_LIBRARY

using void_t = std::void_t;

#else

template <typename... Ts>
struct make_void {
    typedef void type;
};
template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

#endif

template <typename F, typename... Args>
using invoke_result_ =
    decltype(invoke(std::declval<F>(), std::declval<Args>()...));

template <typename F, typename... Args>
struct invoke_nothrow_ : bool_constant<noexcept(invoke(
                             std::declval<F>(), std::declval<Args>()...))> {};

//  from: http://en.cppreference.com/w/cpp/types/result_of, CC-BY-SA

template <typename Void, typename F, typename... Args>
struct invoke_result {};

template <typename F, typename... Args>
struct invoke_result<void_t<invoke_result_<F, Args...> >, F, Args...> {
    using type = invoke_result_<F, Args...>;
};

template <typename Void, typename F, typename... Args>
struct is_invocable : std::false_type {};

template <typename F, typename... Args>
struct is_invocable<void_t<invoke_result_<F, Args...> >, F, Args...>
    : std::true_type {};

template <typename Void, typename R, typename F, typename... Args>
struct is_invocable_r : std::false_type {};

template <typename R, typename F, typename... Args>
struct is_invocable_r<void_t<invoke_result_<F, Args...> >, R, F, Args...>
    : std::is_convertible<invoke_result_<F, Args...>, R> {};

template <typename Void, typename F, typename... Args>
struct is_nothrow_invocable : std::false_type {};

template <typename F, typename... Args>
struct is_nothrow_invocable<void_t<invoke_result_<F, Args...> >, F, Args...>
    : invoke_nothrow_<F, Args...> {};

}  // namespace invoke_detail

//  mimic: std::invoke_result, C++17
template <typename F, typename... Args>
struct invoke_result : invoke_detail::invoke_result<void, F, Args...> {};

//  mimic: std::invoke_result_t, C++17
template <typename F, typename... Args>
using invoke_result_t = typename invoke_result<F, Args...>::type;

//  mimic: std::is_invocable, C++17
template <typename F, typename... Args>
struct is_invocable : invoke_detail::is_invocable<void, F, Args...> {};

//  mimic: std::is_invocable_r, C++17
template <typename R, typename F, typename... Args>
struct is_invocable_r : invoke_detail::is_invocable_r<void, R, F, Args...> {};

//  mimic: std::is_nothrow_invocable, C++17
template <typename F, typename... Args>
struct is_nothrow_invocable
    : invoke_detail::is_nothrow_invocable<void, F, Args...> {};

#endif

}  // namespace async_simple

#endif  // ASYNC_SIMPLE_INVOKE_H
