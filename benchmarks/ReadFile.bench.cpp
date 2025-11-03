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
#include "ReadFile.bench.h"
#include "async_simple/coro/Collect.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/executors/SimpleExecutor.h"
#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
#include "async_simple/uthread/Async.h"
#include "async_simple/uthread/Collect.h"
#endif
#include "ReadFileUtil.hpp"
using namespace async_simple;
using namespace async_simple::coro;
using namespace async_simple::executors;

#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
using namespace async_simple::uthread;
#endif

static int task_num = 1000;

#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
void Uthread_read_diff_size_bench(benchmark::State &state) {
    using namespace async_simple::uthread;
    int id = state.range(0);
    std::string s = std::string("test.") + std::to_string(id) + ".txt";
    gen_file(s, id);
    SimpleExecutor e(std::thread::hardware_concurrency());
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<std::function<void()>> task_list;
        task_list.reserve(task_num);
        for (int i = 0; i < task_num; ++i) {
            task_list.emplace_back(
                [s, &e]() { Uthread_read_file(s.c_str(), &e); });
        }
        state.ResumeTiming();
        std::atomic<bool> running = true;
        async<Launch::Schedule>(
            [&running, &e, task_list]() {
                collectAll<Launch::Schedule>(task_list.begin(), task_list.end(),
                                             &e);
                running = false;
            },
            &e);

        while (running) {
        }
    }
}

void Uthread_same_read_bench(benchmark::State &state) {
    using namespace async_simple::uthread;
    int num = state.range(0);
    int id = 4;
    std::string s = std::string("test.") + std::to_string(id) + ".txt";
    gen_file(s, id);
    auto core_num = std::thread::hardware_concurrency();
    SimpleExecutor e(core_num);
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<std::function<void()>> task_list;
        task_list.reserve(core_num);
        for (size_t i = 0; i < core_num; ++i) {
            task_list.emplace_back([num, &s, e = &e]() mutable {
                Uthread_read_file_for(num, s, e);
            });
        }
        state.ResumeTiming();
        std::atomic<bool> running = true;
        async<Launch::Schedule>(
            [&running, &e, task_list]() {
                collectAll<Launch::Schedule>(task_list.begin(), task_list.end(),
                                             &e);
                running = false;
            },
            &e);

        while (running) {
        }
    }
}
#endif

void Lazy_read_diff_size_bench(benchmark::State &state) {
    using namespace async_simple::coro;
    int id = state.range(0);
    std::string s = std::string("test.") + std::to_string(id) + ".txt";
    gen_file(s, id);
    SimpleExecutor e(std::thread::hardware_concurrency());
    for (auto _ : state) {
        auto f = [&s, &e]() -> Lazy<void> {
            std::vector<Lazy<size_t>> lazy_list;
            lazy_list.reserve(task_num);
            for (int i = 0; i < task_num; ++i) {
                lazy_list.push_back(
                    async_read_file(s.c_str(), e.getIOExecutor()));
            }
            co_await collectAllPara(std::move(lazy_list));
            co_return;
        };
        syncAwait(f().via(&e));
    }
    unlink(s.c_str());
}
void Lazy_same_read_bench(benchmark::State &state) {
    using namespace async_simple::coro;
    int num = state.range(0);
    int id = 4;
    std::string s = std::string("test.") + std::to_string(id) + ".txt";
    gen_file(s, id);
    auto core_num = std::thread::hardware_concurrency();
    SimpleExecutor e(core_num);
    for (auto _ : state) {
        auto f = [&s, &e, num, core_num]() -> Lazy<void> {
            std::vector<Lazy<void>> lazy_list;
            lazy_list.reserve(core_num);
            for (size_t i = 0; i < core_num; ++i) {
                lazy_list.push_back(
                    async_read_file_for(num, s, e.getIOExecutor()));
            }
            co_await collectAllPara(std::move(lazy_list));
            co_return;
        };
        syncAwait(f().via(&e));
    }
    unlink(s.c_str());
}
