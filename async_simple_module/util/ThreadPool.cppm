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
module;
#include <cassert>
export module async_simple:util.ThreadPool;
import :util.Queue;
import std;

namespace async_simple::util {
export class ThreadPool {
public:
    struct WorkItem {
        // Whether or not the fn is auto schedule.
        // If the user don't assign a thread to fn,
        // thread pool will apply random policy to fn.
        // If enable work steal,
        // thread pool will apply work steal policy firstly , if failed, will
        // apply random policy to fn.
        bool autoSchedule = false;
        std::function<void()> fn = nullptr;
    };
    static constexpr std::size_t DEFAULT_THREAD_NUM = 4;
    enum ERROR_TYPE {
        ERROR_NONE = 0,
        ERROR_POOL_ITEM_IS_NULL,
    };

    explicit ThreadPool(std::size_t threadNum = DEFAULT_THREAD_NUM,
                        bool enableWorkSteal = false);
    ~ThreadPool();

    ThreadPool::ERROR_TYPE scheduleById(std::function<void()> fn,
                                        std::size_t id = -1);
    std::size_t getCurrentId() const;
    std::size_t getItemCount() const;
    std::size_t getThreadNum() const { return _threadNum; }

private:
    std::pair<std::size_t, ThreadPool *> *getCurrent() const;
    std::size_t _threadNum;
    std::vector<Queue<WorkItem>> _queues;
    std::vector<std::thread> _threads;
    std::atomic<bool> _stop;
    bool _enableWorkSteal;
};

inline ThreadPool::ThreadPool(std::size_t threadNum, bool enableWorkSteal)
    : _threadNum(threadNum ? threadNum : DEFAULT_THREAD_NUM),
      _queues(_threadNum),
      _stop(false),
      _enableWorkSteal(enableWorkSteal) {
    auto worker = [this](std::size_t id) {
        auto current = getCurrent();
        current->first = id;
        current->second = this;
        while (true) {
            WorkItem workerItem = {};
            if (_enableWorkSteal) {
                // Try to do work steal firstly.
                for (auto n = 0; n < _threadNum * 2; ++n) {
                    if (_queues[(id + n) % _threadNum].try_pop(
                            workerItem,
                            [](auto &item) { return item.autoSchedule; }))
                        break;
                }
            }

            if (!workerItem.fn && !_queues[id].pop(workerItem))
                break;

            if (workerItem.fn) {
                workerItem.fn();
            }
        }
    };

    _threads.reserve(_threadNum);
    for (std::size_t i = 0; i < _threadNum; ++i) {
        _threads.emplace_back(worker, i);
    }
}

inline ThreadPool::~ThreadPool() {
    _stop = true;
    for (auto &queue : _queues)
        queue.stop();
    for (auto &thread : _threads)
        thread.join();
}

inline ThreadPool::ERROR_TYPE ThreadPool::scheduleById(std::function<void()> fn,
                                                       std::size_t id) {
    if (nullptr == fn)
        return ERROR_POOL_ITEM_IS_NULL;

    if (id == -1) {
        if (_enableWorkSteal) {
            // Try to push to a non-block queue firstly.
            WorkItem workerItem{false, fn};
            for (auto n = 0; n < _threadNum * 2; ++n) {
                if (_queues[(id + n) % _threadNum].try_push(workerItem))
                    return ERROR_NONE;
            }
        }

        id = std::rand() % _threadNum;
        _queues[id].push(WorkItem{false, std::move(fn)});
    } else {
        assert(id < _threadNum);
        _queues[id].push(WorkItem{true, std::move(fn)});
    }

    return ERROR_NONE;
}

inline std::pair<std::size_t, ThreadPool *> *ThreadPool::getCurrent() const {
    static thread_local std::pair<std::size_t, ThreadPool *> current(-1, nullptr);
    return &current;
}

inline std::size_t ThreadPool::getCurrentId() const {
    auto current = getCurrent();
    if (this == current->second) {
        return current->first;
    }
    return -1;
}

inline std::size_t ThreadPool::getItemCount() const {
    std::size_t ret = 0;
    for (std::size_t i = 0; i < _threadNum; ++i) {
        ret += _queues[i].size();
    }
    return ret;
}
}  // namespace async_simple::util