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
#include "ThreadPool.bench.h"
#include <atomic>
#include <cassert>

using namespace async_simple::util;

const int COUNT = 500'000;
const int REPS = 10;

void autoScheduleTasks(bool enableWorkSteal) {
    std::atomic<int> count = 0;
    ThreadPool tp(std::thread::hardware_concurrency(), enableWorkSteal);

    for (int i = 0; i < COUNT; ++i) {
        [[maybe_unused]] auto ret = tp.scheduleById([i, &count] {
            count++;
            int x;
            auto reps = REPS + (REPS * (rand() % 5));
            for (int n = 0; n < reps; ++n)
                x = i + rand();
            (void)x;
        });
        assert(ret == ThreadPool::ERROR_TYPE::ERROR_NONE);
    }
}

void ThreadPool_noWorkSteal(benchmark::State& state) {
    for ([[maybe_unused]] const auto& _ : state)
        autoScheduleTasks(/*enableWorkSteal = */ false);
}

void ThreadPool_withWorkSteal(benchmark::State& state) {
    for ([[maybe_unused]] const auto& _ : state)
        autoScheduleTasks(/*enableWorkSteal = */ true);
}
