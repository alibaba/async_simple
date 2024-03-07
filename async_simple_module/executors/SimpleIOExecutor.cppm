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
export module async_simple:executors.SimpleIOExecutor;
import :IOExecutor;
import std;

export namespace async_simple {

namespace executors {
// This is a demo IOExecutor.
//  submitIO and submitIOV should be implemented.
class SimpleIOExecutor : public IOExecutor {
public:
    static constexpr int kMaxAio = 8;

public:
    SimpleIOExecutor() {}
    virtual ~SimpleIOExecutor() {}

    SimpleIOExecutor(const IOExecutor&) = delete;
    SimpleIOExecutor& operator=(const IOExecutor&) = delete;

public:
    class Task {
    public:
        Task(AIOCallback& func) : _func(func) {}
        ~Task() {}

    public:
        void process(io_event& event) { _func(event); }

    private:
        AIOCallback _func;
    };

public:
    void submitIO(int fd, iocb_cmd cmd, void* buffer, std::size_t length,
                  std::size_t offset, AIOCallback cbfn) override {
        _shutdown = true;
    }

private:
    volatile bool _shutdown = false;
    std::thread _loopThread;
};

}  // namespace executors

}  // namespace async_simple
