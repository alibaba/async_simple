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

#include <condition_variable>
#include <deque>
#include <mutex>
#include "async_simple/Executor.h"

namespace async_simple {

namespace executors {

class LocalExecutor : public Executor {
public:
    LocalExecutor(std::condition_variable& cv, std::mutex& mut,
                  const bool& quit)
        : Executor("local executor"), cv_(cv), mut_(mut), quit_(quit) {}

    bool schedule(Func func) override {
        std::unique_lock<std::mutex> lock(mut_);
        ready_queue_.push_back(std::move(func));
        cv_.notify_all();
        return true;
    }

    void Loop() {
        while (true) {
            std::unique_lock<std::mutex> lock(mut_);
            cv_.wait(lock, [this]() {
                return quit_ == true || !ready_queue_.empty();
            });
            if (!ready_queue_.empty()) {
                Func func = std::move(ready_queue_.front());
                ready_queue_.pop_front();
                lock.unlock();
                func();
                continue;
            }
            if (quit_ == true) {
                return;
            }
        }
    }

private:
    std::deque<Func> ready_queue_;
    std::condition_variable& cv_;
    std::mutex& mut_;
    const bool& quit_;
};

}  // namespace executors

}  // namespace async_simple

#endif
