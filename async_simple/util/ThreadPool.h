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
/* The file implements a simple thread pool. The scheduling strategy is a simple
 * random strategy. Simply, ThreadPool would create n threads. And for a task
 * which is waiting to be scheduled. The ThreadPool would choose a thread for
 * this task randomly.
 *
 * The purpose of ThreadPool is for testing. People who want to use async_simple
 * in actual development should implement/use more complex executor.
 */
#ifndef FUTURE_THREAD_POOL_H
#define FUTURE_THREAD_POOL_H

#include <atomic>
#include <chrono>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

namespace async_simple {

namespace util {

class ThreadMutex {
public:
    ThreadMutex(const pthread_mutexattr_t *mta = NULL) {
        pthread_mutex_init(&_mutex, mta);
    }

    ~ThreadMutex() { pthread_mutex_destroy(&_mutex); }

    int lock() { return pthread_mutex_lock(&_mutex); }

    int trylock() { return pthread_mutex_trylock(&_mutex); }

    int unlock() { return pthread_mutex_unlock(&_mutex); }

private:
    ThreadMutex(const ThreadMutex &);
    ThreadMutex &operator=(const ThreadMutex &);

protected:
    pthread_mutex_t _mutex;
};

class ProducerConsumerCond : public ThreadMutex {
public:
    ProducerConsumerCond() {
        pthread_cond_init(&_producerCond, NULL);
        pthread_cond_init(&_consumerCond, NULL);
    }

    ~ProducerConsumerCond() {
        pthread_cond_destroy(&_producerCond);
        pthread_cond_destroy(&_consumerCond);
    }

public:
    int producerWait(int64_t usec = 0) { return wait(_producerCond, usec); }

    int signalProducer() { return signal(_producerCond); }

    int broadcastProducer() { return broadcast(_producerCond); }

    int consumerWait(int64_t usec = 0) { return wait(_consumerCond, usec); }

    int signalConsumer() { return signal(_consumerCond); }

    int broadcastConsumer() { return broadcast(_consumerCond); }

private:
    int wait(pthread_cond_t &cond, int64_t usec) {
        int ret = 0;
        assert(usec == 0);
        ret = pthread_cond_wait(&cond, &_mutex);
        return ret;
    }

    static int signal(pthread_cond_t &cond) {
        return pthread_cond_signal(&cond);
    }

    static int broadcast(pthread_cond_t &cond) {
        return pthread_cond_broadcast(&cond);
    }

protected:
    pthread_cond_t _producerCond;
    pthread_cond_t _consumerCond;
};

class ScopedLock {
private:
    ThreadMutex &_lock;

private:
    ScopedLock(const ScopedLock &);
    ScopedLock &operator=(const ScopedLock &);

public:
    explicit ScopedLock(ThreadMutex &lock) : _lock(lock) {
        int ret = _lock.lock();
        assert(ret == 0);
        (void)ret;
    }

    ~ScopedLock() {
        int ret = _lock.unlock();
        assert(ret == 0);
        (void)ret;
    }
};

class WorkItem {
public:
    ~WorkItem() {
        //        destroy();
    }

    WorkItem(std::function<void()> rhs) { fn = std::move(rhs); }

    WorkItem(WorkItem &&rhs) { fn = std::move(rhs.fn); }

public:
    void process() { fn(); }
    void destroy() { delete this; }
    void drop() { delete this; }

private:
    std::function<void()> fn;
};

class ThreadPool {
public:
    enum {
        DEFAULT_THREADNUM = 4,
    };

    enum STOP_TYPE {
        STOP_THREAD_ONLY = 0,
        STOP_AND_CLEAR_QUEUE,
        STOP_AFTER_QUEUE_EMPTY
    };

    enum ERROR_TYPE {
        ERROR_NONE = 0,
        ERROR_POOL_HAS_STOP,
        ERROR_POOL_ITEM_IS_NULL,
        ERROR_POOL_QUEUE_FULL,
    };

public:
    ThreadPool(size_t threadNum = DEFAULT_THREADNUM);
    ~ThreadPool();

private:
    ThreadPool(const ThreadPool &);
    ThreadPool &operator=(const ThreadPool &);

public:
    ERROR_TYPE scheduleById(std::function<void()> fn, int32_t id = -1);
    int32_t getCurrentId() const;
    size_t getItemCount() const;
    size_t getThreadNum() const { return _threadNum; }
    bool start();
    void stop(STOP_TYPE stopType = STOP_AFTER_QUEUE_EMPTY);
    void clearQueue();
    size_t getActiveThreadNum() const;
    const std::vector<std::thread> &getThreads() const { return _threads; }

private:
    void push(WorkItem *item);
    void pushFront(WorkItem *item);
    bool tryPopItem(WorkItem *&);
    bool createThreads();
    void workerLoop(size_t id);
    void waitQueueEmpty();
    void stopThread();
    std::pair<int32_t, ThreadPool *> *getCurrent() const;

private:
    size_t _threadNum;
    std::vector<std::queue<WorkItem *> > _queue;
    std::vector<std::thread> _threads;
    mutable std::vector<ProducerConsumerCond> _cond;
    volatile bool _push;
    volatile bool _run;
    std::atomic<std::uint32_t> _activeThreadCount;

private:
    friend class ThreadPoolTest;
};

typedef std::shared_ptr<ThreadPool> ThreadPoolPtr;

inline ThreadPool::ThreadPool(size_t threadNum)
    : _threadNum(threadNum),
      _queue(threadNum ? threadNum : DEFAULT_THREADNUM),
      _cond(threadNum ? threadNum : DEFAULT_THREADNUM),
      _push(true),
      _run(false),
      _activeThreadCount(0) {
    if (_threadNum == 0) {
        _threadNum = DEFAULT_THREADNUM;
    }
}

inline ThreadPool::~ThreadPool() { stop(STOP_AND_CLEAR_QUEUE); }

inline size_t ThreadPool::getItemCount() const {
    size_t ret = 0;
    for (size_t i = 0; i < _threadNum; ++i) {
        ScopedLock lock(_cond[i]);
        ret += _queue[i].size();
    }
    return ret;
}

inline ThreadPool::ERROR_TYPE ThreadPool::scheduleById(std::function<void()> fn,
                                                       int32_t id) {
    if (-1 == id) {
        id = rand() % _threadNum;
    }
    assert(id >= 0 && static_cast<size_t>(id) < _threadNum);
    WorkItem *item = new WorkItem(std::move(fn));
    if (!_push) {
        return ERROR_POOL_HAS_STOP;
    }

    if (NULL == item) {
        return ERROR_POOL_ITEM_IS_NULL;
    }

    ScopedLock lock(_cond[id]);

    if (!_push) {
        return ERROR_POOL_HAS_STOP;
    }

    _queue[id].push(item);
    _cond[id].signalConsumer();
    return ERROR_NONE;
}

inline bool ThreadPool::start() {
    if (_run) {
        return false;
    }

    _push = true;
    _run = true;

    if (createThreads()) {
        return true;
    } else {
        stop(STOP_THREAD_ONLY);
        return false;
    }
}

inline void ThreadPool::stop(STOP_TYPE stopType) {
    if (STOP_THREAD_ONLY != stopType) {
        { _push = false; }
    }

    if (STOP_AFTER_QUEUE_EMPTY == stopType) {
        waitQueueEmpty();
    }

    stopThread();

    if (STOP_THREAD_ONLY != stopType) {
        clearQueue();
    }
}

inline void ThreadPool::stopThread() {
    if (!_run) {
        return;
    }
    {
        for (size_t i = 0; i < _threadNum; ++i) {
            ScopedLock lock(_cond[i]);
            _run = false;
            _cond[i].broadcastConsumer();
        }
    }
    for (auto &it : _threads) {
        it.join();
    }
    _threads.clear();
}

inline std::pair<int32_t, ThreadPool *> *ThreadPool::getCurrent() const {
    static thread_local std::pair<int32_t, ThreadPool *> current(-1, nullptr);
    return &current;
}

inline void ThreadPool::waitQueueEmpty() {
    using namespace std::chrono_literals;
    while (true) {
        if ((size_t)0 == getItemCount()) {
            break;
        }
        std::this_thread::sleep_for(10000us);
    }
}

inline bool ThreadPool::createThreads() {
    size_t num = _threadNum;
    while (num--) {
        _threads.push_back(
            std::thread(std::bind(&ThreadPool::workerLoop, this, num)));
    }
    return true;
}

inline void ThreadPool::clearQueue() {
    for (size_t i = 0; i < _threadNum; ++i) {
        ScopedLock lock(_cond[i]);
        while (!_queue[i].empty()) {
            WorkItem *item = _queue[i].front();
            _queue[i].pop();
            if (item) {
                item->drop();
            }
        }
        _cond[i].broadcastProducer();
    }
}

inline int32_t ThreadPool::getCurrentId() const {
    auto current = getCurrent();
    if (this == current->second) {
        return current->first;
    }
    return -1;
}

inline void ThreadPool::workerLoop(size_t id) {
    auto current = getCurrent();
    current->first = id;
    current->second = this;
    while (_run) {
        WorkItem *item = NULL;
        {
            ScopedLock lock(_cond[id]);
            while (_run && _queue[id].empty()) {
                _cond[id].consumerWait();
            }

            if (!_run) {
                return;
            }

            item = _queue[id].front();
            _queue[id].pop();
            _cond[id].signalProducer();
            if (item) {
                ++_activeThreadCount;
            }
        }

        if (item) {
            item->process();
            item->destroy();
            --_activeThreadCount;
        }
    }
}

inline size_t ThreadPool::getActiveThreadNum() const {
    return _activeThreadCount;
}

}  // namespace util

}  // namespace async_simple

#endif  // FUTURE_THREAD_POOL_H
