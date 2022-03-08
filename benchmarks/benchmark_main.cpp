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
#include "Uthread.bench.h"

BENCHMARK(Future_chain);
BENCHMARK(Future_collectAll);
BENCHMARK(async_simple_Lazy_chain);
BENCHMARK(async_simple_Lazy_collectAll);
BENCHMARK(RescheduleLazy_chain);
BENCHMARK(RescheduleLazy_collectAll);
BENCHMARK(Uthread_switch);
BENCHMARK(Uthread_async);
BENCHMARK(Uthread_await);
BENCHMARK(Uthread_collectAll);
BENCHMARK_MAIN();
