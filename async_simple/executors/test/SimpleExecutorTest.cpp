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

#include <gtest/gtest.h>

#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <type_traits>

TEST(SimpleExecutorTest, testNormal) {
    using namespace async_simple::executors;
    SimpleExecutor ex(2);

    std::condition_variable cv;
    std::mutex mut;
    size_t done_count = 0;
    size_t sum = 0;

    ex.schedule([&sum, &done_count, &mut, &cv]() {
        std::unique_lock<std::mutex> guard(mut);
        done_count += 1;
        sum += 10;
        cv.notify_one();
    });

    auto tmp = std::make_unique<int>(20);
    auto move_only_functor = [&sum, &done_count, &mut, &cv,
                              tmp = std::move(tmp)] {
        std::unique_lock<std::mutex> guard(mut);
        sum += *tmp;
        done_count += 1;
        cv.notify_one();
    };

    EXPECT_FALSE(
        std::is_copy_constructible<decltype(move_only_functor)>::value);
    ex.schedule_move_only(
        async_simple::util::move_only_function(std::move(move_only_functor)));
    std::unique_lock<std::mutex> guard(mut);
    cv.wait(guard, [&]() { return done_count == 2; });
    EXPECT_EQ(sum, 30);
}
