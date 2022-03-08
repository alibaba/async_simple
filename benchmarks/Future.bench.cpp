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

#include "async_simple/executors/SimpleExecutor.h"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

using namespace async_simple;

void Future_chain(benchmark::State& state) {
    auto test = []() {
        async_simple::executors::SimpleExecutor executor(10);
        Promise<int> p;
        int output0 = 0;
        int output1 = 0;
        int output2 = 0;
        std::vector<int> order;
        std::mutex mtx;
        auto record = [&order, &mtx](int x) {
            std::lock_guard<std::mutex> l(mtx);
            order.push_back(x);
        };
        auto f = p.getFuture()
                     .via(&executor)
                     .thenTry([&output0, record](Try<int>&& t) {
                         record(0);
                         output0 = t.value();
                         return t.value() + 100;
                     })
                     .thenTry([&output1, &executor, record](Try<int>&& t) {
                         record(1);
                         output1 = t.value();
                         Promise<int> p;
                         auto f = p.getFuture().via(&executor);
                         p.setValue(t.value() + 10);
                         return f;
                     })
                     .thenValue([&output2, record](int x) {
                         record(2);
                         output2 = x;
                         return std::to_string(x);
                     })
                     .thenValue(
                         [](std::string&& s) { return std::stoi(s) + 1111.0; });
    };

    for ([[maybe_unused]] const auto& _ : state)
        test();
}
void Future_collectAll(benchmark::State& state) {
    auto test = []() {
        size_t n = 5000;

        async_simple::executors::SimpleExecutor executor(10);
        std::vector<Promise<int>> promise(n);
        std::vector<Future<int>> futures;

        for (size_t i = 0; i < n; ++i)
            futures.push_back(promise[i].getFuture().via(&executor));

        std::vector<int> expected{0};

        auto f =
            collectAll(futures.begin(), futures.end())
                .thenValue([&expected](std::vector<Try<int>>&& vec) {
                    for (size_t i = 0; i < vec.size(); ++i)
                        expected.push_back(expected.back() + vec[i].value());
                });

        for (size_t i = 0; i < n; ++i)
            promise[i].setValue(i);

        f.wait();
    };

    for ([[maybe_unused]] const auto& _ : state)
        test();
}
