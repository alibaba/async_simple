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

#ifndef ASYNC_LOCAL_EXECUTOR_H
#define ASYNC_LOCAL_EXECUTOR_H

#include <deque>
#include "async_simple/Executor.h"

namespace async_simple {

namespace executors {

class LocalExecutor : public Executor {
public:
    LocalExecutor() : Executor("local executor") {}

    bool schedule(Func func) override {
        ready_queue_.push_back(std::move(func));
        return true;
    }

    void Loop() {
        while (true) {
            if (ready_queue_.empty()) {
                return;
            }
            Func func = std::move(ready_queue_.front());
            ready_queue_.pop_front();
            func();
        }
    }

private:
    std::deque<Func> ready_queue_;
};

}  // namespace executors

}  // namespace async_simple

#endif
