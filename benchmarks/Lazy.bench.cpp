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
#include "Lazy.bench.h"

#include "async_simple/executors/SimpleExecutor.h"

template <template <typename> typename LazyType, int N>
struct lazy_fn {
    LazyType<int> operator()() {
        co_return N + co_await lazy_fn<LazyType, N - 1>()();
    }
};

template <template <typename> typename LazyType>
struct lazy_fn<LazyType, 0> {
    LazyType<int> operator()() { co_return 1; }
};

void async_simple_Lazy_chain(benchmark::State& state) {
    auto chain_starter = [&]() -> async_simple::coro::Lazy<int> {
        co_return co_await lazy_fn<async_simple::coro::Lazy, 1000>()();
    };
    for ([[maybe_unused]] const auto& _ : state)
        async_simple::coro::syncAwait(chain_starter());
}

void async_simple_Lazy_collectAll(benchmark::State& state) {
    auto collectAllStarter = [&]() -> async_simple::coro::Lazy<void> {
        std::vector<async_simple::coro::Lazy<int>> lazies;
        for (int i = 0; i < 5000; i++)
            lazies.push_back(lazy_fn<async_simple::coro::Lazy, 50>()());
        co_await async_simple::coro::collectAllPara(std::move(lazies));
    };
    for ([[maybe_unused]] const auto& _ : state)
        syncAwait(collectAllStarter());
}

void RescheduleLazy_chain(benchmark::State& state) {
    auto chain_starter = [&]() -> async_simple::coro::Lazy<int> {
        co_return co_await lazy_fn<async_simple::coro::Lazy, 1000>()();
    };
    async_simple::executors::SimpleExecutor e(10);
    for ([[maybe_unused]] const auto& _ : state)
        async_simple::coro::syncAwait(chain_starter().via(&e));
}

void RescheduleLazy_collectAll(benchmark::State& state) {
    auto collectAllStarter = [&]() -> async_simple::coro::Lazy<void> {
        std::vector<async_simple::coro::Lazy<int>> lazies;
        for (int i = 0; i < 5000; i++)
            lazies.push_back(lazy_fn<async_simple::coro::Lazy, 50>()());
        co_await async_simple::coro::collectAllPara(std::move(lazies));
    };
    async_simple::executors::SimpleExecutor e(10);
    for ([[maybe_unused]] const auto& _ : state)
        syncAwait(collectAllStarter().via(&e));
}
