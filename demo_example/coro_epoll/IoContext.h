//
// Created by xmh on 25-2-13.
//

#ifndef IOCONTEXT_H
#define IOCONTEXT_H

#include <sys/epoll.h>
#include "Socket.h"
#include "async_simple/executors/SimpleExecutor.h"

class Socket;

class IoContext {
public:
    IoContext(int maxThreads = 10, int maxEvents = 100)
        : maxThreads_(maxThreads), maxEvents_(maxEvents) {
        executor_ = new async_simple::executors::SimpleExecutor(maxThreads_);
        eventPool_ = new epoll_event[maxEvents_];
        epollFd_ = epoll_create1(0);
        if (epollFd_ == -1) {
            std::cerr << "Error creating epoll!" << std::endl;
        }
    }

    IoContext(const IoContext &other) = delete;

    IoContext &operator=(const IoContext &other) = delete;

    IoContext(IoContext &&other) {
        epollFd_ = std::exchange(other.epollFd_, -1);
        maxEvents_ = std::exchange(other.maxEvents_, 0);
        maxThreads_ = std::exchange(other.maxThreads_, 0);
        executor_ = std::exchange(other.executor_, nullptr);
        eventPool_ = std::exchange(other.eventPool_, nullptr);
    }

    IoContext &operator=(IoContext &&other) {
        std::swap(epollFd_, other.epollFd_);
        std::swap(maxThreads_, other.maxThreads_);
        std::swap(maxEvents_, other.maxEvents_);
        std::swap(executor_, other.executor_);
        std::swap(eventPool_, other.eventPool_);
        return *this;
    }

    ~IoContext() {
        delete executor_;
        delete[] eventPool_;
    }

    void run() {
        while (true) {
            int nfds = epoll_wait(epollFd_, eventPool_, maxEvents_, -1);
            for (int i = 0; i < nfds; ++i) {
                auto socket = static_cast<Socket *>(eventPool_[i].data.ptr);
                const auto events = eventPool_[i].events;

                socket->current_state_ |= (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP));
                socket->current_state_ &= (events & (EPOLLIN | EPOLLOUT | EPOLLET | EPOLLONESHOT));
                // 隐式监听
                if (events & (EPOLLERR | EPOLLHUP)) {
                    ::close(socket->fd_);
                    socket->fd_ = -1;
                    if (socket->coro_send_) {

                    }
                    // if (socket->isReadSuspending.exchange(false, std::memory_order_relaxed)) {
                    //     executor_->schedule([&socket] {
                    //         socket->coro_recv_.resume();
                    //     });
                    // }
                    // if (socket->isWriteSuspending.exchange(false, std::memory_order_relaxed)) {
                    //     executor_->schedule([&socket] {
                    //         socket->coro_send_.resume();
                    //     });
                    // }
                    continue;
                }

                if ((events & EPOLLIN) && socket->isReadSuspending.exchange(false, std::memory_order_relaxed)) {
                    executor_->schedule([&socket] {
                        socket->coro_recv_.resume();
                    });
                }

                if ((events & EPOLLOUT) && socket->isWriteSuspending.exchange(false, std::memory_order_relaxed)) {
                    executor_->schedule([&socket] {
                        socket->coro_send_.resume();
                    });
                }
            }
        }
    }

public:
    int epollFd_;
    int maxThreads_;
    int maxEvents_;
    async_simple::executors::SimpleExecutor *executor_;
    epoll_event *eventPool_;
};

#endif  // IOCONTEXT_H
