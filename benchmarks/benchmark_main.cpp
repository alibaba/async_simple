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
#include "Future.bench.h"
#include "Lazy.bench.h"
#include "ThreadPool.bench.h"
#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
#include "Uthread.bench.h"
#endif
#include "CallDepth.bench.h"
#include "PureSwitch.bench.h"
#include "ReadFile.bench.h"

BENCHMARK(Future_chain);
BENCHMARK(Future_collectAll);
BENCHMARK(async_simple_Lazy_chain);
BENCHMARK(async_simple_Lazy_collectAll);
BENCHMARK(RescheduleLazy_chain);
BENCHMARK(RescheduleLazy_collectAll);
#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
BENCHMARK(Uthread_switch);
BENCHMARK(Uthread_async);
BENCHMARK(Uthread_await);
BENCHMARK(Uthread_collectAll);
BENCHMARK(Uthread_pure_switch_bench)
    ->RangeMultiplier(10)
    ->Range(1000 * 100, 1000 * 1000 * 100)
    ->Iterations(3);
BENCHMARK(Uthread_read_diff_size_bench)
    ->RangeMultiplier(2)
    ->Range(2 << 4, 2 << 10)
    ->Iterations(3);
BENCHMARK(Uthread_same_read_bench)
    ->RangeMultiplier(2)
    ->Range(2 << 4, 2 << 10)
    ->Iterations(3);
#endif
BENCHMARK(ThreadPool_noWorkSteal);
BENCHMARK(ThreadPool_withWorkSteal);
BENCHMARK(Generator_pure_switch_bench)
    ->RangeMultiplier(10)
    ->Range(1000 * 100, 1000 * 1000 * 100)
    ->Iterations(3);
BENCHMARK(Lazy_read_diff_size_bench)
    ->RangeMultiplier(2)
    ->Range(2 << 4, 2 << 10)
    ->Iterations(3);
BENCHMARK(Lazy_same_read_bench)
    ->RangeMultiplier(2)
    ->Range(2 << 4, 2 << 10)
    ->Iterations(3);
#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
BENCHMARK(Uthread_call_depth_bench)
    ->RangeMultiplier(2)
    ->Range(1, 2 << 10)
    ->Iterations(3);
#endif
BENCHMARK(Lazy_call_depth_bench)
    ->RangeMultiplier(2)
    ->Range(1, 2 << 10)
    ->Iterations(3);
BENCHMARK_MAIN();
