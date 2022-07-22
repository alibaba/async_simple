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
/* This file implements the collectAll interface. The collectAll interface would
 * await all functions in a specified range. It would create a uthread for every
 * function in the range and await for every uthread to complete.
 * uthread::collectAll should be called in a uthread only. Generally, when
 * uthread::collectAll is called, the current uthread would check out until all
 * of the corresponding awaited uthread finished. Then the current uthread would
 * check in.
 *
 * The available schedule policy includes Launch::Schedule and Launch::Current.
 * See Async.h for details.
 *
 * Example:
 * ```C++
 *  std::vector<Callable> v;
 *  make tasks concurrent execution.
 *  auto res1 = collectAll<Launch::Schedule>(v.begin(), v.end(), ex);
 *
 *  make tasks async execution in current thread.
 *  auto res2 = collectAll<Launch::Current>(v.begin(), v.end(), ex);
 * ```
 *
 * the type of res is std::vector<T>, the T is user task's return type.
 */
#ifndef ASYNC_SIMPLE_UTHREAD_COLLECT_H
#define ASYNC_SIMPLE_UTHREAD_COLLECT_H

#include <async_simple/Future.h>
#include <async_simple/uthread/Async.h>
#include <async_simple/uthread/Await.h>
#include <type_traits>

namespace async_simple {
namespace uthread {

// TODO: Due to it is possible that the user of async_simple doesn't support
// c++17, we didn't merge this two implementation by if constexpr. Merge them
// once the codebases are ready to use c++17.
template <class Policy, class Iterator>
std::vector<typename std::enable_if<
    !std::is_void<std::invoke_result_t<
        typename std::iterator_traits<Iterator>::value_type>>::value,
    std::invoke_result_t<typename std::iterator_traits<Iterator>::value_type>>::
                type>
collectAll(Iterator first, Iterator last, Executor* ex) {
    assert(std::distance(first, last) >= 0);
    static_assert(!std::is_same<Launch::Prompt, Policy>::value,
                  "collectAll not support Prompt launch policy");

    using ResultType = std::invoke_result_t<
        typename std::iterator_traits<Iterator>::value_type>;

    struct Context {
        std::atomic<std::size_t> tasks;
        std::vector<ResultType> result;
        Promise<std::vector<ResultType>> promise;

        Context(std::size_t n, Promise<std::vector<ResultType>>&& pr)
            : tasks(n), result(n), promise(pr) {}
    };

    return await<std::vector<ResultType>>(
        ex, [first, last, ex](Promise<std::vector<ResultType>>&& pr) mutable {
            auto n = static_cast<std::size_t>(std::distance(first, last));
            auto context = std::make_shared<Context>(n, std::move(pr));
            for (auto i = 0; first != last; ++i, ++first) {
                async<Policy>(
                    [context, i, f = std::move(*first)]() mutable {
                        context->result[i] = std::move(f());
                        auto lastTasks = context->tasks.fetch_sub(
                            1u, std::memory_order_acq_rel);
                        if (lastTasks == 1u) {
                            context->promise.setValue(
                                std::move(context->result));
                        }
                    },
                    ex);
            }
        });
}

template <class Policy, class Iterator>
typename std::enable_if<
    std::is_void<std::invoke_result_t<
        typename std::iterator_traits<Iterator>::value_type>>::value,
    void>::type
collectAll(Iterator first, Iterator last, Executor* ex) {
    assert(std::distance(first, last) >= 0);
    static_assert(!std::is_same<Launch::Prompt, Policy>::value,
                  "collectN not support Prompt launch policy");

    struct Context {
        std::atomic<std::size_t> tasks;
        Promise<bool> promise;

        Context(std::size_t n, Promise<bool>&& pr) : tasks(n), promise(pr) {}
    };

    await<bool>(ex, [first, last, ex](Promise<bool>&& pr) mutable {
        auto n = static_cast<std::size_t>(std::distance(first, last));
        auto context = std::make_shared<Context>(n, std::move(pr));
        for (; first != last; ++first) {
            async<Policy>(
                [context, f = std::move(*first)]() mutable {
                    f();
                    auto lastTasks =
                        context->tasks.fetch_sub(1u, std::memory_order_acq_rel);
                    if (lastTasks == 1u) {
                        context->promise.setValue(true);
                    }
                },
                ex);
        }
    });
}

}  // namespace uthread
}  // namespace async_simple

#endif  // ASYNC_SIMPLE_UTHREAD_COLLECT_H