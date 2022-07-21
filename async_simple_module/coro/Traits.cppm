
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
export module async_simple:coro.Traits;
import std;

namespace async_simple {

namespace coro {

namespace detail {

template <typename T>
class HasCoAwaitMethod {
    template <typename C>
    static std::int8_t test(decltype(std::declval<C>().coAwait(nullptr))*);

    template <typename C>
    static std::int16_t test(...);

public:
    static constexpr bool value = (sizeof(test<T>(nullptr)) == sizeof(std::int8_t));
};

template <typename T>
class HasMemberCoAwaitOperator {
    template <typename C>
    static std::int8_t test(decltype(std::declval<C>().operator co_await())*);
    template <typename C>
    static std::int16_t test(...);

public:
    static constexpr bool value = (sizeof(test<T>(nullptr)) == sizeof(std::int8_t));
};

template <typename T>
class HasGlobalCoAwaitOperator {
    template <typename C>
    static std::int8_t test(decltype(operator co_await(std::declval<C>()))*);
    template <typename C>
    static std::int16_t test(...);

public:
    static constexpr bool value = (sizeof(test<T>(nullptr)) == sizeof(std::int8_t));
};

template <
    typename Awaitable,
    std::enable_if_t<HasMemberCoAwaitOperator<std::decay_t<Awaitable>>::value,
                     int> = 0>
auto getAwaiter(Awaitable&& awaitable) {
    return std::forward<Awaitable>(awaitable).operator co_await();
}

template <
    typename Awaitable,
    std::enable_if_t<HasGlobalCoAwaitOperator<std::decay_t<Awaitable>>::value,
                     int> = 0>
auto getAwaiter(Awaitable&& awaitable) {
    return operator co_await(std::forward<Awaitable>(awaitable));
}

template <
    typename Awaitable,
    std::enable_if_t<!HasMemberCoAwaitOperator<std::decay_t<Awaitable>>::value,
                     int> = 0,
    std::enable_if_t<!HasGlobalCoAwaitOperator<std::decay_t<Awaitable>>::value,
                     int> = 0>
auto getAwaiter(Awaitable&& awaitable) {
    return std::forward<Awaitable>(awaitable);
}

}  // namespace detail

}  // namespace coro
}  // namespace async_simple
