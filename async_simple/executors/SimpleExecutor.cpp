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
    [[maybe_unused]] auto ret = _pool.start();
    assert(ret);
    _ioExecutor.init();
}

SimpleExecutor::~SimpleExecutor() {
    _ioExecutor.destroy();
    _pool.stop();
}

SimpleExecutor::Context SimpleExecutor::checkout() {
    return _pool.getCurrentId() | kContextMask;
}

bool SimpleExecutor::checkin(Func func, Context ctx, ScheduleOptions opts) {
    int64_t id = std::get<2>(ctx) & (~kContextMask);
    auto prompt = _pool.getCurrentId() == id && opts.prompt;
    if (prompt) {
        func();
        return true;
    }
    return _pool.scheduleById(std::move(func), id) ==
           util::ThreadPool::ERROR_NONE;
}

}  // namespace executors

}  // namespace async_simple
