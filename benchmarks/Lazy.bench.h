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
#include "async_simple/coro/Lazy.h"
#include "benchmark/benchmark.h"

void async_simple_Lazy_chain(benchmark::State& state);
void async_simple_Lazy_collectAll(benchmark::State& state);
void RescheduleLazy_chain(benchmark::State& state);
void RescheduleLazy_collectAll(benchmark::State& state);
