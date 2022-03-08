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
#include "Uthread.bench.h"

#include <thread>
#include <vector>
#include "async_simple/executors/SimpleExecutor.h"

using namespace std;
using namespace async_simple;
using namespace async_simple::uthread;

template <class Func>
void delayedTask(Executor* ex, Func&& func, std::size_t ms) {
    std::thread(
        [f = std::move(func), ms](Executor* ex) {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            ex->schedule(std::move(f));
        },
        ex)
        .detach();
}

void Uthread_switch(benchmark::State& state) {
    auto test = []() {
        async_simple::executors::SimpleExecutor executor(10);
        auto Job = [&]() -> Future<int> {
            Promise<int> p;
            auto f = p.getFuture().via(&executor);
            delayedTask(
                &executor,
                [p = std::move(p)]() mutable {
                    auto value = 1024;
                    p.setValue(value);
                },
                100);
            return f;
        };

        constexpr size_t n = 10;
        std::atomic<int> running = n;

        auto UthreadTask = [&executor, &running, &Job]() {
            Uthread task(Attribute{&executor}, [&running, &Job]() {
                Job().get();
                running--;
            });
            task.detach();
        };

        for (size_t i = 0; i < n; i++)
            executor.schedule(UthreadTask);

        while (running) {
        }
    };

    for ([[maybe_unused]] const auto& _ : state)
        test();
}

void Uthread_async(benchmark::State& state) {
    auto test = []() {
        async_simple::executors::SimpleExecutor executor(10);
        auto Job = [&]() -> Future<int> {
            Promise<int> p;
            auto f = p.getFuture().via(&executor);
            delayedTask(
                &executor,
                [p = std::move(p)]() mutable {
                    auto value = 1024;
                    p.setValue(value);
                },
                100);
            return f;
        };

        constexpr size_t n = 10;
        std::atomic<int> running = n;

        for (size_t i = 0; i < n; i++)
            async<Launch::Schedule>(
                [&running, &Job]() {
                    Job().get();
                    running--;
                },
                &executor);

        while (running) {
        }
    };

    for ([[maybe_unused]] const auto& _ : state)
        test();
}

void Uthread_await(benchmark::State& state) {
    auto test = []() {
        async_simple::executors::SimpleExecutor executor(10);
        auto Job = [&](Promise<int> p) {
            delayedTask(
                &executor,
                [p = std::move(p)]() mutable {
                    auto value = 1024;
                    p.setValue(value);
                },
                100);
        };

        constexpr size_t n = 10;
        std::atomic<int> running = n;

        for (size_t i = 0; i < n; i++)
            async<Launch::Schedule>(
                [&executor, &running, &Job]() {
                    await<int>(&executor, Job);
                    running--;
                },
                &executor);

        while (running) {
        }
    };

    for ([[maybe_unused]] const auto& _ : state)
        test();
}

void Uthread_collectAll(benchmark::State& state) {
    auto test = []() {
        async_simple::executors::SimpleExecutor executor(10);
        auto Job = [&](std::size_t delay_ms) -> Future<int> {
            Promise<int> p;
            auto f = p.getFuture().via(&executor);
            delayedTask(
                &executor,
                [p = std::move(p)]() mutable {
                    auto value = 1024;
                    p.setValue(value);
                },
                delay_ms);
            return f;
        };

        constexpr size_t n = 10;
        std::vector<std::function<std::size_t()>> Tasks;
        for (size_t i = 0; i < n; ++i)
            Tasks.emplace_back(
                [i, &Job]() -> std::size_t { return i + Job(n - i).get(); });

        std::atomic<bool> running = true;

        async<Launch::Schedule>(
            [&running, &executor, Tasks = std::move(Tasks)]() mutable {
                collectAll<Launch::Schedule>(Tasks.begin(), Tasks.end(),
                                             &executor);
                running = false;
            },
            &executor);

        while (running) {
        }
    };

    for ([[maybe_unused]] const auto& _ : state)
        test();
}
