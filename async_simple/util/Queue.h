// The file implements a simple thread safe queue.
#ifndef ASYNC_SIMPLE_QUEUE_H
#define ASYNC_SIMPLE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace async_simple::util {

template <typename T>
class Queue {
public:
    Queue() = default;

    void push(T &&item) {
        {
            std::scoped_lock guard(_mutex);
            _queue.push(std::move(item));
        }
        _cond.notify_one();
    }

    bool try_push(const T &item) {
        {
            std::unique_lock lock(_mutex, std::try_to_lock);
            if (!lock)
                return false;
            _queue.push(item);
        }
        _cond.notify_one();
        return true;
    }

    bool pop(T &item) {
        std::unique_lock lock(_mutex);
        _cond.wait(lock, [&]() { return !_queue.empty() || _stop; });
        if (_queue.empty())
            return false;
        item = std::move(_queue.front());
        _queue.pop();
        return true;
    }

    // non-blocking pop an item, maybe pop failed.
    // predict is an extension pop condition, default is null.
    bool try_pop(T &item, bool (*predict)(T &) = nullptr) {
        std::unique_lock lock(_mutex, std::try_to_lock);
        if (!lock || _queue.empty())
            return false;

        if (predict && predict(_queue.front())) {
            return false;
        }

        item = std::move(_queue.front());
        _queue.pop();
        return true;
    }

    std::size_t size() const {
        std::scoped_lock guard(_mutex);
        return _queue.size();
    }

    bool empty() const {
        std::scoped_lock guard(_mutex);
        return _queue.empty();
    }

    void stop() {
        {
            std::scoped_lock guard(_mutex);
            _stop = true;
        }
        _cond.notify_all();
    }

private:
    std::queue<T> _queue;
    bool _stop = false;
    mutable std::mutex _mutex;
    std::condition_variable _cond;
};
}  // namespace async_simple::util
#endif  // ASYNC_SIMPLE_QUEUE_H