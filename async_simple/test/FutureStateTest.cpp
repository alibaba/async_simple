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
#include <exception>
#include <functional>
#include "async_simple/FutureState.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

using namespace std;
using namespace async_simple::executors;

namespace async_simple {

class FutureStateTest : public FUTURE_TESTBASE {
public:
    void caseSetUp() override {}
    void caseTearDown() override {}
};

namespace {

enum DummyState : int {
    CONSTRUCTED = 1,
    DESTRUCTED = 2,
};

struct Dummy {
    Dummy() : state(nullptr) {}
    Dummy(int* state_) : state(state_) {
        if (state) {
            (*state) |= CONSTRUCTED;
        }
    }
    Dummy(Dummy&& other) : state(other.state) { other.state = nullptr; }
    Dummy& operator=(Dummy&& other) {
        if (this != &other) {
            std::swap(other.state, state);
        }
        return *this;
    }

    ~Dummy() {
        if (state) {
            (*state) |= DESTRUCTED;
            state = nullptr;
        }
    }

    Dummy(const Dummy&) = delete;
    Dummy& operator=(const Dummy&) = delete;

    int* state = nullptr;
};
}  // namespace

TEST_F(FutureStateTest, testSimpleProcess) {
    auto fs = new FutureState<int>();
    fs->attachOne();
    ASSERT_FALSE(fs->hasResult());
    ASSERT_FALSE(fs->hasContinuation());
    ASSERT_FALSE(fs->getExecutor());

    fs->setResult(Try<int>(100));
    ASSERT_TRUE(fs->hasResult());
    ASSERT_FALSE(fs->hasContinuation());
    auto& v = fs->getTry();
    ASSERT_EQ(100, v.value());

    int output = 0;
    fs->setContinuation(
        [&output](Try<int>&& v) mutable { output = v.value() + 5; });
    ASSERT_TRUE(fs->hasResult());
    ASSERT_TRUE(fs->hasContinuation());

    ASSERT_EQ(105, output);

    // you can not getTry after setContinuation,
    // because value has been moved into continuation
    // auto v = std::move(fs->getTry());
    // ASSERT_EQ(100, v.value());

    fs->detachOne();
}

TEST_F(FutureStateTest, testSimpleExecutor) {
    auto fs = new FutureState<int>();
    fs->attachOne();
    auto executor = new SimpleExecutor(5);
    fs->setExecutor(executor);
    ASSERT_FALSE(fs->hasResult());
    ASSERT_FALSE(fs->hasContinuation());
    ASSERT_TRUE(fs->getExecutor());

    fs->setResult(Try<int>(100));
    ASSERT_TRUE(fs->hasResult());
    ASSERT_FALSE(fs->hasContinuation());

    int output = 0;
    fs->setContinuation(
        [&output](Try<int>&& v) mutable { output = v.value() + 5; });
    ASSERT_TRUE(fs->hasResult());
    ASSERT_TRUE(fs->hasContinuation());

    delete executor;
    ASSERT_EQ(105, output);

    // auto v = std::move(fs->getTry());
    // ASSERT_EQ(100, v.value());
    fs->detachOne();
}

TEST_F(FutureStateTest, testClass) {
    auto fs = new FutureState<Dummy>();
    fs->attachOne();
    auto executor = new SimpleExecutor(5);
    fs->setExecutor(executor);

    ASSERT_FALSE(fs->hasResult());
    ASSERT_FALSE(fs->hasContinuation());
    EXPECT_TRUE(fs->getExecutor());

    int state = 0;
    Try<Dummy> v(&state);
    fs->setResult(std::move(v));
    ASSERT_TRUE(fs->hasResult());
    ASSERT_FALSE(fs->hasContinuation());
    ASSERT_TRUE(fs->getTry().value().state);

    int* output = nullptr;
    Dummy noCopyable(nullptr);
    fs->setContinuation(
        [&output, d = std::move(noCopyable)](Try<Dummy>&& v) mutable {
            auto localV = std::move(v);
            output = localV.value().state;
            (void)d;
        });
    EXPECT_TRUE(fs->hasResult());
    EXPECT_TRUE(fs->hasContinuation());

    delete executor;
    EXPECT_EQ(&state, output);

    EXPECT_FALSE(fs->getTry().value().state);
    fs->detachOne();
}

}  // namespace async_simple
