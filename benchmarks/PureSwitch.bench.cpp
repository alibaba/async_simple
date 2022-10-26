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
#include "PureSwitch.bench.h"
#include <async_simple/executors/SimpleExecutor.h>
#include <async_simple/experimental/coroutine.h>
#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
#define private public
#include <async_simple/uthread/Uthread.h>
#undef private
#include <async_simple/uthread/Async.h>
#endif
using namespace async_simple;
using namespace async_simple::executors;
using namespace async_simple::uthread;
template <typename T>
struct Generator {
    struct promise_type;
    using Handle = std::coroutine_handle<promise_type>;
    Generator(Handle h) : handle_(h) {}
    ~Generator() { handle_.destroy(); }
    struct promise_type {
        T val_;
        auto get_return_object() { return Handle::from_promise(*this); }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        auto yield_value(T val) {
            this->val_ = val;
            return std::suspend_always{};
        }
        void unhandled_exception() {}
        void return_value(T val) { this->val_ = val; }
    };
    T operator()() {
        handle_.resume();
        return handle_.promise().val_;
    }

private:
    Handle handle_;
};
Generator<int> f() {
    [[maybe_unused]] int i = 0;
    while (true) {
        co_yield i;
        i++;
    }
}
void Generator_pure_switch_bench(benchmark::State &state) {
    int num = state.range(0);
    SimpleExecutor e(1);
    for (auto _ : state) {
        auto g = f();
        for (int i = 0; i < num; ++i) {
            benchmark::DoNotOptimize(g());
            // g();
        }
    }
}
#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
void Uthread_pure_switch_bench(benchmark::State &state) {
    int num = state.range(0);
    SimpleExecutor e(1);
    for (auto _ : state) {
        async<Launch::Current>(
            [&e, num]() {
                auto t1 = async<Launch::Prompt>(
                    []() {
                        auto ctx = uthread::internal::thread_impl::get();
                        while (true) {
                            uthread::internal::thread_impl::switch_out(ctx);
                        }
                    },
                    &e);
                for (int i = 0; i < num; ++i) {
                    t1._ctx->switch_in();
                }
            },
            &e);
    }
}
#endif
