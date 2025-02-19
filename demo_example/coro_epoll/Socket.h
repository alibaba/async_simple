//
// Created by xmh on 25-2-13.
//

#ifndef SOCKET_H
#define SOCKET_H

#include <coroutine>
#include <cstdint>
#include <sys/socket.h>
#include <iostream>
#include <atomic>
#include <demo_example/http/http_response.hpp>

#include "IoContext.h"

class IoContext;
class Connect;

// 一个Socket最多可被两个线程操作（一个读一个写）

class Socket {
public:
    // enum class state : uint32_t {
    //     IN = 1, // 可读
    //     OUT = 1<<1, // 可写
    //     ERR = 1<<2, // 发生错误
    //     HUP = 1<<3, // 双方都断开连接
    //     RDHUP = 1<<4, // （对方发送FIN后不再继续发送数据）
    //     ET = 1<<5, // ET模式
    //     ONESHOT = 1<<6, // 只监听一次事件，事件触发之后从epoll移除
    // };
    Socket(int domain, int type, int protocol, uint32_t io_state = (EPOLLIN | EPOLLOUT | EPOLLRDHUP), IoContext *io_context = nullptr) : io_context_(io_context), io_state_(io_state) {
        fd_ = ::socket(domain, type, protocol);
        if (fd_ == -1) {
            std::cerr << "Error creating socket" << std::endl;
            exit(-1);
        }
        if (!addEvent(io_state_) || !attach2IoContext()) {
            std::cerr << "Error attached to iocontext" << std::endl;
            exit(-1);
        }
    }

    Socket(const Socket &) = delete;

    Socket &operator=(const Socket &) = delete;

    Socket(Socket &&other) = delete;

    Socket &operator=(Socket &&other) = delete;

    ~Socket() {
        if (fd_ != -1) {
            fd_ = -1;
            ::close(fd_);
        }
    }

    bool attach2IoContext() {
        if (!io_context_ || io_context_->epollFd_ == -1 || io_context_->maxEvents_ <= 0 || io_context_->maxThreads_ <= 0)
            return false;
        const int epoll_fd = io_context_->epollFd_;
        epoll_event event{};
        event.events = io_state_;
        event.data.ptr = this;
        const int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd_, &event);
        return (ret != -1);
    }

    bool addEvent(const uint32_t event) {
        auto old_io_state = io_state_.load(std::memory_order_relaxed);
        auto new_io_state{};
        do {
            new_io_state = old_io_state | event;
        }while (!io_state_.compare_exchange_strong(old_io_state, new_io_state, std::memory_order_relaxed, std::memory_order_relaxed));
        return true;
    }

    bool removeEvent(uint32_t event) {

    }

public:
    friend class IoContext;
    friend class Connect;

public:
    int fd_ = -1;
    IoContext *io_context_ = nullptr;
    // 当前监听的事件，epoll需要用modify所以需要将旧的事件保存起来
    std::atomic<uint32_t> io_state_;

    // 实际监听到的事件
    uint32_t current_state_ = 0;
    // 读协程是否在suspending
    std::atomic<bool> isReadSuspending{false};
    // 写协程是否在suspending
    std::atomic<bool> isWriteSuspending{false};

    // 因为可能有两个协程同时在等待一个socket，所以要用两个coroutine_handle来保存。
    std::coroutine_handle<> coro_recv_{nullptr}; // 接收数据的协程
    std::coroutine_handle<> coro_send_{nullptr}; // 发送数据的协程
};

struct sendAwaiter {
    Socket* sock;
    sendAwaiter(Socket* s) : sock(s){}
    bool await_ready() noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h) noexcept {
        sock->coro_send_ = h;
    }

    auto await_resume() noexcept {
        auto h = sock->coro_send_;

    }
};


#endif //SOCKET_H
