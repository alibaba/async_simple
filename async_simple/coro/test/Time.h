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
#ifndef ASYNC_SIMPLE_TEST_TIME_H
#define ASYNC_SIMPLE_TEST_TIME_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std::chrono;
class ScopeRuntime {
public:
    explicit ScopeRuntime(const std::string& msg, int loop)
        : _msg(msg), _loop(loop) {
        auto d = steady_clock::now().time_since_epoch();
        auto mic = duration_cast<nanoseconds>(d);
        _start_time = mic.count();
    }
    ~ScopeRuntime() {
        auto d = steady_clock::now().time_since_epoch();
        auto mic = duration_cast<nanoseconds>(d);
        long time_ns = ((mic.count() - _start_time) / _loop);
        double time_us = time_ns / 1000.0;
        double time_ms = time_us / 1000.0;
        if (time_ms > 100) {
            std::cout << std::right << std::setw(30) << _msg.data() << ": "
                      << time_ms << " ms" << std::endl;
        } else if (time_us > 100) {
            std::cout << std::right << std::setw(30) << _msg.data() << ": "
                      << time_us << " us" << std::endl;
        } else {
            std::cout << std::right << std::setw(30) << _msg.data() << ": "
                      << time_ns << " ns" << std::endl;
        }
    }
    int64_t _start_time;
    std::string _msg;
    int _loop;
};

#endif
