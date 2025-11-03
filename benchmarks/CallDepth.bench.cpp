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
#include "CallDepth.bench.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
#include "async_simple/uthread/Async.h"
#endif
#include "ReadFileUtil.hpp"
using namespace async_simple;
using namespace async_simple::coro;
using namespace async_simple::executors;

#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
using namespace async_simple::uthread;
#endif

static int core_num = 1;

int sum(int n) {
    if (n == 0) {
        return 0;
    }
    return n + sum(n - 1);
}

Lazy<int> lazy_sum(int n) {
    if (n == 0) {
        co_return 0;
    }
    co_return n + co_await lazy_sum(n - 1);
}

#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
void Uthread_call_depth_bench(benchmark::State &state) {
    using namespace async_simple::uthread;
    int n = state.range(0);
    SimpleExecutor e(core_num);
    for (auto _ : state) {
        std::atomic<bool> running = true;
        async<Launch::Current>(
            [&running, n]() mutable {
                sum(n);
                running = false;
            },
            &e);
        while (running) {
        }
    }
}
#endif

void Lazy_call_depth_bench(benchmark::State &state) {
    using namespace async_simple::coro;
    int n = state.range(0);
    SimpleExecutor e(core_num);
    for (auto _ : state) {
        syncAwait(lazy_sum(n));
    }
}
