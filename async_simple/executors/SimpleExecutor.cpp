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
#include <async_simple/executors/SimpleExecutor.h>

namespace async_simple {

namespace executors {

// 0xBFFFFFFF == ~0x40000000
static constexpr int64_t kContextMask = 0x40000000;

SimpleExecutor::SimpleExecutor(size_t threadNum) : _pool(threadNum) {
    _ioExecutor.init();
}

SimpleExecutor::~SimpleExecutor() { _ioExecutor.destroy(); }

SimpleExecutor::Context SimpleExecutor::checkout() {
    // avoid CurrentId equal to NULLCTX
    return reinterpret_cast<Context>(_pool.getCurrentId() | kContextMask);
}

bool SimpleExecutor::checkin(Func func, Context ctx, ScheduleOptions opts) {
    int64_t id = reinterpret_cast<int64_t>(ctx);
    auto prompt = _pool.getCurrentId() == (id & (~kContextMask)) && opts.prompt;
    if (prompt) {
        func();
        return true;
    }
    return _pool.scheduleById(std::move(func), id & (~kContextMask)) ==
           util::ThreadPool::ERROR_NONE;
}

}  // namespace executors

}  // namespace async_simple
