//
// Created by xmh on 25-2-13.
//

#ifndef ASYNC_SIMPLE_HOOK_SYS_CALL_H
#define ASYNC_SIMPLE_HOOK_SYS_CALL_H

#include <async_simple/coro/FutureAwaiter.h>
#include <bits/cxxabi_tweaks.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include "Socket.h"
#include "CurrentCoroutine.h"
#include "async_simple/coro/Lazy.h"

// 假设Socket::fd_已经是no_block模式
async_simple::coro::Lazy<int> connect(Socket &sock, sockaddr &serverAdder) {
    int ret = ::connect(sock.fd_, &serverAdder, sizeof(serverAdder));
    if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        auto current_handle = co_await CurrentCoroutine{};
        sock.coro_send_ = current_handle;
        sock.isWriteSuspending.store(true, std::memory_order_relaxed);
        if ((sock.io_state_ & EPOLLOUT) || sock.addEvent(EPOLLOUT)) {
            co_await std::suspend_always{};
            if (sock.fd_ != -1) {
                // TODO 判断是否是ET模式
                ret = ::connect(sock.fd_, &serverAdder, sizeof(serverAdder));
            }
        }
    }
    co_return ret;
}

async_simple::coro::Lazy<int> send(Socket &sock, void *buffer, size_t len) {
    int ret = ::send(sock.fd_, buffer, len, 0);
    if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        auto current_handle = co_await CurrentCoroutine{};
        sock.coro_send_ = current_handle;
        sock.isWriteSuspending.store(true, std::memory_order_relaxed);
        if ((sock.io_state_ & EPOLLOUT) || sock.addEvent(EPOLLOUT)) {
            co_await std::suspend_always{};
            if (sock.fd_ != -1) {
                // TODO 判断是否是ET模式
                ret = ::send(sock.fd_, buffer, len, 0);
            }
        }
    }
    co_return ret;
}

async_simple::coro::Lazy<int> recv(Socket &sock, void *buffer, size_t len) {
    int ret = ::recv(sock.fd_, buffer, len, 0);
    if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        auto current_handle = co_await CurrentCoroutine{};
        sock.coro_recv_ = current_handle;
        sock.isReadSuspending.store(true, std::memory_order_relaxed);
        if ((sock.io_state_ & EPOLLIN) || sock.addEvent(EPOLLIN)) {
            co_await std::suspend_always{};
            if (sock.fd_ != -1) {
                // TODO 判断是否是ET模式
                // TODO 出错时关闭fd
                ret = ::recv(sock.fd_, buffer, len, 0);
            }
        }
    }
    co_return ret;
}

async_simple::coro::Lazy<int> accept(Socket &sock) {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    int ret = ::accept(sock.fd_, (sockaddr *) (&addr), &len);
    if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        auto current_handle = co_await CurrentCoroutine{};
        sock.coro_send_ = current_handle;
        sock.isReadSuspending.store(true, std::memory_order_relaxed);
        if ((sock.io_state_ & EPOLLOUT) || sock.addEvent(EPOLLOUT)) {
            co_await std::suspend_always{};
            if (sock.fd_ != -1) {
                // TODO 判断是否是ET模式
                // TODO 出错时关闭fd
                ret = ::accept(sock.fd_, (sockaddr *) (&addr), &len);
            }
        }
    }
    co_return ret;
}

#endif  // ASYNC_SIMPLE_HOOK_SYS_CALL_H
