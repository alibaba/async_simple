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
#include "SharedLock.bench.h"

#include "async_simple/coro/Collect.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SharedLock.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"

#include <random>
#include <set>
#include <vector>

using namespace async_simple;
using namespace async_simple::coro;

void SharedLock_ReadMoreThanWrite_chain(benchmark::State& state) {
    SharedLock m;
    executors::SimpleExecutor executor(8);

    std::default_random_engine e;
    std::uniform_int_distribution<int> rnd(0, 1000000);
    std::atomic<int64_t> sum = 0;
    std::set<int> table;
    for (int i = 0; i < 1000000; ++i) {
        table.insert(rnd(e));
    }
    auto tester = [&]() {
        const int writer_count = 100;
        const int reader_count = 10000;
        const int read_count_max = 100;

        auto writer = [&]() -> Lazy<void> {
            co_await m.coLock();
            table.erase(rnd(e));
            co_await m.unlock();
        };
        auto reader = [&]() -> Lazy<void> {
            co_await m.coLockShared();
            for (int i = 0; i < read_count_max; ++i) {
                auto iter = table.find(rnd(e));
                if (iter != table.end()) {
                    sum += *iter;
                }
            }
            co_await m.unlockShared();
        };
        std::vector<Lazy<void>> works;

        for (int i = 0; i < reader_count; ++i)
            works.emplace_back(reader());
        for (int i = 0; i < writer_count; ++i)
            works.emplace_back(writer());
        auto f = [works_ = std::move(works)]() mutable -> Lazy<void> {
            [[maybe_unused]] auto ret =
                co_await collectAllPara(std::move(works_));
            co_return;
        };
        syncAwait(f().via(&executor));
    };
    for ([[maybe_unused]] const auto& _ : state)
        tester();
    std::cout << sum << std::endl;
}

void SpinLock_ReadMoreThanWrite_chain(benchmark::State& state) {
    SpinLock m;
    executors::SimpleExecutor executor(8);

    std::default_random_engine e;
    std::uniform_int_distribution<int> rnd(0, 1000000);
    std::atomic<int64_t> sum = 0;
    std::set<int> table;
    for (int i = 0; i < 1000000; ++i) {
        table.insert(rnd(e));
    }
    auto tester = [&]() {
        const int writer_count = 100;
        const int reader_count = 10000;
        const int read_count_max = 100;

        auto writer = [&]() -> Lazy<void> {
            co_await m.coLock();
            table.erase(rnd(e));
            m.unlock();
        };
        auto reader = [&]() -> Lazy<void> {
            co_await m.coLock();
            for (int i = 0; i < read_count_max; ++i) {
                auto iter = table.find(rnd(e));
                if (iter != table.end()) {
                    sum += *iter;
                }
            }
            m.unlockShared();
        };
        std::vector<Lazy<void>> works;

        for (int i = 0; i < reader_count; ++i)
            works.emplace_back(reader());
        for (int i = 0; i < writer_count; ++i)
            works.emplace_back(writer());
        auto f = [works_ = std::move(works)]() mutable -> Lazy<void> {
            [[maybe_unused]] auto ret =
                co_await collectAllPara(std::move(works_));
            co_return;
        };
        syncAwait(f().via(&executor));
    };
    for ([[maybe_unused]] const auto& _ : state)
        tester();
    std::cout << sum << std::endl;
}