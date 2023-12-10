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

#include <gtest/gtest.h>

#include <memory>

#include "async_simple/util/move_only_function.h"

namespace async_simple {

namespace util {

TEST(move_only_function, empty) {
    move_only_function<int(int)> f;
    EXPECT_THROW(f(2), std::bad_function_call);
}

int test_for_move_only(int i) { return i; }

int test_for_move_only_2(int i) { return i + 1; }

TEST(move_only_function, function) {
    move_only_function<int(int)> f(test_for_move_only);
    EXPECT_EQ(f(2), 2);
    f = move_only_function<int(int)>(test_for_move_only_2);
    EXPECT_EQ(f(2), 3);
    f = nullptr;
    EXPECT_EQ(f.operator bool(), false);
    EXPECT_THROW(f(2), std::bad_function_call);
    move_only_function<int(int)> f3(&test_for_move_only);
    EXPECT_EQ(f3(2), 2);
}

struct TestForMoveOnlyFunction {
    TestForMoveOnlyFunction(int i) : i_(i) {}

    TestForMoveOnlyFunction(const TestForMoveOnlyFunction& other) {
        i_ = other.i_;
        copy_constructor_count += 1;
    }

    TestForMoveOnlyFunction(TestForMoveOnlyFunction&& other) {
        i_ = other.i_;
        move_cosntructor_count += 1;
    }

    TestForMoveOnlyFunction& operator=(const TestForMoveOnlyFunction& other) {
        i_ = other.i_;
        copy_assign_count += 1;
        return *this;
    }

    TestForMoveOnlyFunction& operator=(TestForMoveOnlyFunction&& other) {
        i_ = other.i_;
        move_assign_count += 1;
        return *this;
    }

    ~TestForMoveOnlyFunction() { destructor_count += 1; }

    int i_ = 0;

    int operator()() { return i_; }

    static int copy_constructor_count;
    static int copy_assign_count;

    static int move_cosntructor_count;
    static int move_assign_count;
    static int destructor_count;

    static void reset_count() {
        copy_constructor_count = 0;
        copy_assign_count = 0;
        move_cosntructor_count = 0;
        move_assign_count = 0;
        destructor_count = 0;
    }
};

int TestForMoveOnlyFunction::copy_constructor_count = 0;
int TestForMoveOnlyFunction::copy_assign_count = 0;
int TestForMoveOnlyFunction::move_cosntructor_count = 0;
int TestForMoveOnlyFunction::move_assign_count = 0;
int TestForMoveOnlyFunction::destructor_count = 0;

TEST(move_only_function, functor) {
    move_only_function<int()> f(TestForMoveOnlyFunction{10});
    EXPECT_EQ(TestForMoveOnlyFunction::copy_assign_count, 0);
    EXPECT_EQ(TestForMoveOnlyFunction::copy_constructor_count, 0);
    EXPECT_EQ(TestForMoveOnlyFunction::move_assign_count, 0);
    EXPECT_EQ(TestForMoveOnlyFunction::move_cosntructor_count, 1);
    EXPECT_EQ(f(), 10);
    TestForMoveOnlyFunction::reset_count();

    TestForMoveOnlyFunction tmp{20};
    move_only_function<int()> f2(tmp);
    EXPECT_EQ(TestForMoveOnlyFunction::copy_assign_count, 0);
    EXPECT_EQ(TestForMoveOnlyFunction::copy_constructor_count, 1);
    EXPECT_EQ(TestForMoveOnlyFunction::move_assign_count, 0);
    EXPECT_EQ(TestForMoveOnlyFunction::move_cosntructor_count, 0);
    EXPECT_EQ(f2(), 20);
    TestForMoveOnlyFunction::reset_count();

    move_only_function<int()> f3(std::move(tmp));
    EXPECT_EQ(TestForMoveOnlyFunction::copy_assign_count, 0);
    EXPECT_EQ(TestForMoveOnlyFunction::copy_constructor_count, 0);
    EXPECT_EQ(TestForMoveOnlyFunction::move_assign_count, 0);
    EXPECT_EQ(TestForMoveOnlyFunction::move_cosntructor_count, 1);
    EXPECT_EQ(f3(), 20);
    TestForMoveOnlyFunction::reset_count();
    f3 = nullptr;
    EXPECT_EQ(TestForMoveOnlyFunction::destructor_count, 1);
}

TEST(move_only_function, lambda) {
    std::unique_ptr<int> num = std::make_unique<int>(10);
    auto lambda = [num = std::move(num)]() -> int { return *num; };
    move_only_function<int()> f(std::move(lambda));
    EXPECT_EQ(f(), 10);

    std::unique_ptr<int> num2(new int(20));
    auto lambda2 = [num2 = std::move(num2)]() -> int { return *num2; };
    move_only_function<int()> f2;
    f2 = std::move(lambda2);
    EXPECT_EQ(f2(), 20);
}

TEST(move_only_function, std_function) {
    std::function<int(int)> f = [](int i) { return i + i; };
    move_only_function f2(f);
    EXPECT_EQ(f2(2), 4);
    f2 = std::function<int(int)>([](int i) { return i * 3; });
    EXPECT_EQ(f2(2), 6);
}

struct TestForMemFnCalledByMoveOnlyFunction {
    int add(int i) const { return i + 10; }
};

TEST(move_only_function, mem_fn) {
    move_only_function<int(const TestForMemFnCalledByMoveOnlyFunction&, int)> f(
        &TestForMemFnCalledByMoveOnlyFunction::add);
    TestForMemFnCalledByMoveOnlyFunction obj;
    EXPECT_EQ(f(obj, 10), 20);
}

TEST(move_only_function, ret_type_check) {
    move_only_function<void(int)> f(test_for_move_only);
    EXPECT_NO_THROW(f(10));
}

TEST(move_only_function, equal_check) {
    move_only_function<void()> f;
    EXPECT_EQ(f, nullptr);
    EXPECT_EQ(nullptr, f);
    move_only_function<void()> f2(nullptr);
    EXPECT_EQ(f2, nullptr);
    f = []() {};
    EXPECT_NE(f, nullptr);
    EXPECT_NE(nullptr, f);
}

}  // namespace util

}  // namespace async_simple
