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
/* This file implements async interface. The call to async
 * would create a uthread stackful coroutine and the corresponding
 * root function in the uthread stackful coroutine.
 *
 * Here are 3 modes available when creating a uthread.
 * - Launch::Prompt. This mode would create a uthread and return the uthread to
 * the caller.
 *   The caller could set a callback by `Uthread::join` API. `Uthread::join` API
 *   would call the callback after the root function ends.
 * - Launch::Schedule. In this mode, it depends on the exeuctor to schedule the
 * task to create
 *   the uthread. So in this mode, the executor passed in shouldn't be nullptr.
 *   In this mode, the user could choose to offer a callback.
 * - Launch::Current. This mode would create a uthread and detach it. So the
 * user wouldn't get the
 *   uthread instance. So that the user couldn't set a callback nor know whether
 *   or not if the uthread finished.
 */
#ifndef ASYNC_SIMPLE_UTHREAD_ASYNC_H
#define ASYNC_SIMPLE_UTHREAD_ASYNC_H

#include <async_simple/uthread/Uthread.h>
#include <memory>
#include <type_traits>

namespace async_simple {
namespace uthread {

struct Launch {
    struct Prompt {};
    struct Schedule {};
    struct Current {};
};

template <class T, class F,
          typename std::enable_if<std::is_same<T, Launch::Prompt>::value,
                                  T>::type* = nullptr>
inline Uthread async(F&& f, Executor* ex) {
    return Uthread(Attribute{ex}, std::forward<F>(f));
}

template <class T, class F,
          typename std::enable_if<std::is_same<T, Launch::Schedule>::value,
                                  T>::type* = nullptr>
inline void async(F&& f, Executor* ex) {
    if (!ex)
        AS_UNLIKELY { return; }
    ex->schedule([f = std::move(f), ex]() {
        Uthread uth(Attribute{ex}, std::move(f));
        uth.detach();
    });
}

// schedule async task, set a callback
template <class T, class F, class C,
          typename std::enable_if<std::is_same<T, Launch::Schedule>::value,
                                  T>::type* = nullptr>
inline void async(F&& f, C&& c, Executor* ex) {
    if (!ex)
        AS_UNLIKELY { return; }
    ex->schedule([f = std::move(f), c = std::move(c), ex]() {
        Uthread uth(Attribute{ex}, std::move(f));
        uth.join(std::move(c));
    });
}

template <class T, class F,
          typename std::enable_if<std::is_same<T, Launch::Current>::value,
                                  T>::type* = nullptr>
inline void async(F&& f, Executor* ex) {
    Uthread uth(Attribute{ex}, std::forward<F>(f));
    uth.detach();
}

}  // namespace uthread
}  // namespace async_simple

#endif  // ASYNC_SIMPLE_UTHREAD_ASYNC_H