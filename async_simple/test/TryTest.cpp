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
#include "async_simple/Try.h"
#include "async_simple/test/unittest.h"

using namespace std;

namespace async_simple {

class TryTest : public FUTURE_TESTBASE {
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
    Dummy() = default;
    Dummy(int* state_) : state(state_) {
        if (state) {
            *state |= CONSTRUCTED;
        }
    }
    Dummy(Dummy&& other) : state(other.state) { other.state = nullptr; }
    Dummy& operator=(Dummy&& other) {
        std::swap(other.state, state);
        return *this;
    }
    ~Dummy() {
        if (state) {
            *state |= DESTRUCTED;
            state = nullptr;
        }
    }
    int* state = nullptr;
};
}  // namespace

TEST_F(TryTest, testSimpleProcess) {
    Try<int> v0(1);
    ASSERT_EQ(1, v0.value());

    Try<int> v1 = 1;
    ASSERT_EQ(1, v1.value());

    Try<int> v2 = std::move(v0);
    ASSERT_EQ(1, v2.value());

    Try<int> v3(std::move(v1));
    ASSERT_TRUE(v3.available());
    ASSERT_FALSE(v3.hasError());
    ASSERT_EQ(1, v3.value());

    Try<int> v4;
    ASSERT_FALSE(v4.available());

    bool hasException = false;
    Try<int> ve;
    try {
        throw "abcdefg";
    } catch (...) {
        ve = std::current_exception();
    }

    ASSERT_TRUE(ve.available());
    ASSERT_TRUE(ve.hasError());

    try {
        ve.value();
    } catch (...) {
        hasException = true;
    }
    ASSERT_TRUE(hasException);

    Try<int> emptyV;
    ASSERT_FALSE(emptyV.available());
    emptyV = 100;
    ASSERT_TRUE(emptyV.available());
    ASSERT_FALSE(emptyV.hasError());
    ASSERT_EQ(100, emptyV.value());
}

TEST_F(TryTest, testClass) {
    int state0 = 0;
    Try<Dummy> v0{Dummy(&state0)};
    EXPECT_TRUE(v0.available());
    EXPECT_FALSE(v0.hasError());
    EXPECT_TRUE(state0 & CONSTRUCTED);
    EXPECT_FALSE(state0 & DESTRUCTED);
    std::exception_ptr error;
    v0 = error;
    EXPECT_TRUE(v0.hasError());
    EXPECT_TRUE(state0 & CONSTRUCTED);
    EXPECT_TRUE(state0 & DESTRUCTED);
}

TEST_F(TryTest, testVoid) {
    Try<void> v;
    bool hasException = false;
    std::exception_ptr error;
    v.setException(std::make_exception_ptr(std::runtime_error("")));
    try {
        v.value();
    } catch (...) {
        hasException = true;
        error = std::current_exception();
    }
    ASSERT_TRUE(hasException);
    ASSERT_TRUE(v.hasError());

    Try<void> ve = error;
    ASSERT_TRUE(ve.hasError());
}

TEST_F(TryTest, testConstruct) {
    Try t1(1);
    static_assert(std::is_same_v<decltype(t1), Try<int>>);
    Try t2(2.0);
    static_assert(std::is_same_v<decltype(t2), Try<double>>);
    std::string hello = "hello";
    Try t3(std::move(hello));
    static_assert(std::is_same_v<decltype(t3), Try<std::string>>);

    Try<std::pair<int, int>> t4(1, 2);
    auto [i, j] = t4.value();
    ASSERT_EQ(i, 1);
    ASSERT_EQ(j, 2);

    struct A {
        int opcode;
        std::string data;
        A(int opcode, const std::string& data) : opcode(opcode), data(data) {}
    };
    Try<A> t5(5, "world");
    auto [opcode, data] = t5.value();
    ASSERT_EQ(opcode, 5);
    ASSERT_EQ(data, "world");
}

}  // namespace async_simple
