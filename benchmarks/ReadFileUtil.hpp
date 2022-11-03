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

#include <async_simple/coro/Collect.h>
#include <async_simple/coro/FutureAwaiter.h>
#include <async_simple/coro/Lazy.h>
#include <async_simple/coro/SyncAwait.h>
#include <async_simple/executors/SimpleExecutor.h>
#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
#include <async_simple/uthread/Async.h>
#include <async_simple/uthread/Collect.h>
#include <async_simple/uthread/Uthread.h>
#endif
#include <fcntl.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <filesystem>
#include <fstream>
using namespace async_simple;
using namespace async_simple::coro;
using namespace async_simple::executors;
namespace fs = std::filesystem;

#ifdef ASYNC_SIMPLE_BENCHMARK_UTHREAD
using namespace async_simple::uthread;

template <typename FileDescriptor, typename Buffer, typename Executor>
std::size_t Uthread_read_some(FileDescriptor fd, Buffer buffer,
                              std::size_t buffer_size, std::size_t offset,
                              Executor *e) {
    Promise<std::size_t> p;

    e->getIOExecutor()->submitIO(
        fd, IOCB_CMD_PREAD, buffer, buffer_size, offset,
        [&p](io_event_t event) mutable { p.setValue(event.res); });
    return uthread::await(p.getFuture().via(e));
}

inline std::size_t Uthread_read_file(const char *filename, SimpleExecutor *e) {
    auto buffer_size = fs::file_size(filename);
    char *buffer = new char[buffer_size];
    auto fd = open(filename, O_RDONLY);
    auto sz = Uthread_read_some(fd, buffer, buffer_size, 0, e);
    delete[] buffer;
    close(fd);
    return sz;
}

void Uthread_read_file_for(int num, const std::string &s, auto e) {
    std::vector<std::function<void()>> task_list;
    task_list.reserve(num);
    for (int i = 0; i < num; ++i) {
        task_list.template emplace_back(
            [&s, e]() { Uthread_read_file(s.c_str(), e); });
    }
    collectAll<Launch::Schedule>(task_list.begin(), task_list.end(), e);
}
#endif

template <typename FileDescriptor, typename Buffer, typename Executor>
Lazy<std::size_t> async_read_some(FileDescriptor fd, Buffer buffer,
                                  int buffer_size, int offset, Executor *e) {
    Promise<std::size_t> p;
    e->submitIO(fd, IOCB_CMD_PREAD, buffer, buffer_size, 0,
                [&p](io_event_t event) { p.setValue(event.res); });
    co_return co_await p.getFuture();
}

inline Lazy<std::size_t> async_read_file(const char *filename, IOExecutor *e) {
    auto buffer_size = fs::file_size(filename);
    char *buffer = new char[buffer_size];
#ifdef _WIN32
    int fd = -1;
#else
    auto fd = open(filename, O_RDONLY);
#endif
    auto size = co_await async_read_some(fd, buffer, buffer_size, 0, e);
    delete[] buffer;
#ifndef _WIN32
    close(fd);
#endif
    co_return size;
}

inline Lazy<void> async_read_file_for(int num, const std::string &s,
                                      IOExecutor *e) {
    std::vector<Lazy<std::size_t>> lazy_list;
    lazy_list.reserve(num);
    for (int i = 0; i < num; ++i) {
        lazy_list.push_back(async_read_file(s.c_str(), e));
    }
    co_await collectAllPara(std::move(lazy_list));
    co_return;
}

inline void gen_file(const std::string &filename, int i) {
    std::fstream f;
    f.open(filename, std::ios::out);
    std::string data;
    data.resize(1024 * i);
    std::fill(data.begin(), data.end(), 'a' + (i % 26));
    f << data;
    f.close();
}
