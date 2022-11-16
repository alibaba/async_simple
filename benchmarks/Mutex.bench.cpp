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
#include "Mutex.bench.h"

#include "async_simple/coro/Collect.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/Mutex.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"

#include <vector>

void Mutex_chain(benchmark::State& state) {
    async_simple::executors::SimpleExecutor executor(50);
    async_simple::coro::Mutex m;
    int value = 0;
    int loop_num{200};
    std::vector<async_simple::coro::RescheduleLazy<void>> input;
    input.reserve(loop_num);
    auto test = [&]() -> async_simple::coro::Lazy<void> {
        auto writer = [&]() -> async_simple::coro::Lazy<void> {
            co_await m.coLock();
            value++;
            m.unlock();
        };
        for (int i = 0; i < loop_num; i++)
            input.emplace_back(writer().via(&executor));
        auto combined_lazy = collectAll(std::move(input));
        auto out = co_await std::move(combined_lazy);
        input.clear();
    };
    for ([[maybe_unused]] const auto& _ : state)
        async_simple::coro::syncAwait(test());
}
