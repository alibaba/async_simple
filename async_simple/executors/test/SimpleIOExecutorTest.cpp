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
#ifndef ASYNC_SIMPLE_HAS_NOT_AIO
#include <fcntl.h>
#include <gtest/gtest.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <exception>
#include <memory>

#include "async_simple/executors/SimpleIOExecutor.h"
#include "async_simple/test/unittest.h"

using namespace std::chrono_literals;

namespace async_simple {

using namespace async_simple::executors;

class SimpleIOExecutorTest : public FUTURE_TESTBASE {
public:
    static constexpr int32_t kBufferSize = 4096 * 2;
    static constexpr auto kTestFile = "/tmp/async_simple_io_test.tmp";

public:
    void caseSetUp() override {
        _ioExecutor = std::make_shared<SimpleIOExecutor>();
        ASSERT_TRUE(_ioExecutor->init());
        _executor = _ioExecutor.get();
    }

    void caseTearDown() override { _ioExecutor->destroy(); }

public:
    std::shared_ptr<SimpleIOExecutor> _ioExecutor;
    IOExecutor* _executor;
};

TEST_F(SimpleIOExecutorTest, testNormal) {
    std::string expect(4096, '0');
    auto fd = open(kTestFile, O_RDWR | O_DIRECT | O_CREAT, 0600);
    auto output = memalign(4096, kBufferSize);
    memcpy((char*)output, expect.data(), expect.length());
    _executor->submitIO(
        fd, IOCB_CMD_PWRITE, output, expect.length(), 0,
        [](io_event_t& event) mutable { EXPECT_EQ(4096, (int32_t)event.res); });
    std::this_thread::sleep_for(300ms);
    memset(output, 0, kBufferSize);
    _executor->submitIO(fd, IOCB_CMD_PREAD, output, kBufferSize, 0,
                        [&expect, output](io_event_t event) mutable {
                            EXPECT_EQ(4096, (int32_t)event.res);
                            EXPECT_EQ(expect,
                                      std::string((char*)output, event.res));
                        });
    std::this_thread::sleep_for(300ms);
    close(fd);
    free(output);
    unlink(kTestFile);
}

TEST_F(SimpleIOExecutorTest, testException) {
    auto output = memalign(4096, kBufferSize);
    memset(output, 0, kBufferSize);
    _executor->submitIO(
        -1, IOCB_CMD_PREAD, output, kBufferSize, 0,
        [](io_event_t& event) mutable { EXPECT_TRUE((int32_t)event.res < 0); });
    std::this_thread::sleep_for(300ms);
    free(output);
}

}  // namespace async_simple
#endif
