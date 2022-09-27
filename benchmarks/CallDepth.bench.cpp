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
#include <async_simple/coro/Collect.h>
#include <async_simple/coro/Lazy.h>
#include <async_simple/coro/SyncAwait.h>
#include <async_simple/executors/SimpleExecutor.h>
#include <async_simple/uthread/Async.h>
#include <async_simple/uthread/Collect.h>
#include <async_simple/uthread/Uthread.h>
#include "ReadFileUtil.hpp"
using namespace async_simple;
using namespace async_simple::coro;
using namespace async_simple::executors;
using namespace async_simple::uthread;
namespace fs = std::filesystem;
static int task_num = 10;
struct Service {
    void run() { service1(); }

    void service1() {
        n--;
        if (n == 0) {
            read_db();
            return;
        }
        int call_idx = n % 2;
        if (call_idx == 0) {
            return service2();
        } else {
            return service3();
        }
    }

    void service2() {
        n--;
        if (n == 0) {
            read_db();
            return;
        }
        int call_idx = n % 2;
        if (call_idx == 0) {
            service1();
        } else {
            return service3();
        }
    }

    void service3() {
        n--;
        if (n == 0) {
            read_db();
            return;
        }
        int call_idx = n % 2;
        if (call_idx == 0) {
            return service1();
        } else {
            return service2();
        }
    }

    void read_db() {
        using namespace async_simple::uthread;
        std::vector<std::function<void()>> task_list;
        for (int i = 0; i < m; ++i) {
            task_list.emplace_back(
                [this]() { Uthread_read_file(filename_, e_); });
        }
        collectAll<Launch::Schedule>(task_list.begin(), task_list.end(), e_);
    }

    int n;
    int m;
    const char *filename_;
    SimpleExecutor *e_;
};

struct AsyncService {
    Lazy<uint64_t> run(const char *filename, int n, IOExecutor *e, int m) {
        co_return co_await service1(filename, n, e, m);
    }

    Lazy<uint64_t> service1(const char *filename, int n, IOExecutor *e, int m) {
        n--;
        if (n == 0) {
            co_return co_await read_db(filename, e, m);
        }
        int call_idx = n % 2;
        if (call_idx == 0) {
            co_return co_await service2(filename, n, e, m);
        } else {
            co_return co_await service3(filename, n, e, m);
        }
    }

    Lazy<uint64_t> service2(const char *filename, int n, IOExecutor *e, int m) {
        n--;
        if (n == 0) {
            co_return co_await read_db(filename, e, m);
        }
        int call_idx = n % 2;
        if (call_idx == 0) {
            co_return co_await service1(filename, n, e, m);
        } else {
            co_return co_await service3(filename, n, e, m);
        }
    }

    Lazy<uint64_t> service3(const char *filename, int n, IOExecutor *e, int m) {
        n--;
        if (n == 0) {
            co_return co_await read_db(filename, e, m);
        }
        int call_idx = n % 2;
        if (call_idx == 0) {
            co_return co_await service1(filename, n, e, m);
        } else {
            co_return co_await service2(filename, n, e, m);
        }
    }

    Lazy<uint64_t> read_db(const char *filename, IOExecutor *e, int m) {
        std::vector<Lazy<std::size_t>> lazy_list;
        for (int i = 0; i < m; i++) {
            lazy_list.push_back(async_read_file(filename, e));
        }
        [[maybe_unused]] auto ret =
            co_await collectAllPara(std::move(lazy_list));
        co_return 0;
    }
};

void Uthread_call_depth_bench(benchmark::State &state) {
    using namespace async_simple::uthread;
    int n = state.range(0);
    std::string s = std::string("test.") + std::to_string(4) + ".txt";
    gen_file(s, 4);
    auto core_num = std::thread::hardware_concurrency();
    SimpleExecutor e(core_num);
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<std::function<void()>> task_list;
        task_list.reserve(task_num);
        std::atomic<bool> running = true;
        int m = 1;
        for (int i = 0; i < task_num; ++i) {
            task_list.emplace_back([n, m, s, e = &e]() {
                Service service{
                    .n = n, .m = m, .filename_ = s.c_str(), .e_ = e};
                service.run();
            });
        }
        state.ResumeTiming();
        async<Launch::Schedule>(
            [&running, &e, task_list]() mutable {
                collectAll<Launch::Schedule>(task_list.begin(), task_list.end(),
                                             &e);
                running = false;
            },
            &e);
        while (running) {
        }
    }
}

void Lazy_call_depth_bench(benchmark::State &state) {
    using namespace async_simple::coro;
    int n = state.range(0);
    auto file_size = 4;
    std::string s = std::string("test.") + std::to_string(file_size) + ".txt";
    gen_file(s, file_size);
    auto core_num = std::thread::hardware_concurrency();
    SimpleExecutor e(core_num);
    for (auto _ : state) {
        auto f = [&s, n, e = &e]() mutable -> Lazy<void> {
            std::vector<Lazy<uint64_t>> lazy_list;
            lazy_list.reserve(task_num);
            int m = 1;
            for (int i = 0; i < task_num; ++i) {
                lazy_list.push_back(
                    AsyncService().run(s.c_str(), n, e->getIOExecutor(), m));
            }
            [[maybe_unused]] auto ret =
                co_await collectAllPara(std::move(lazy_list));
            co_return;
        };
        syncAwait(collectAllPara(f().via(&e)));
    }
    unlink(s.c_str());
}
