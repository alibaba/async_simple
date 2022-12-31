/*
 * Copyright (c) 2023, Alibaba Group Holding Limited;
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

#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>
#include <vector>
#include "async_simple/coro/Generator.h"
#include "async_simple/coro/Lazy.h"
#include "async_simple/test/unittest.h"

namespace async_simple::coro {

class GeneratorTest : public FUTURE_TESTBASE {
public:
    GeneratorTest() = default;
    void caseSetUp() override {}
    void caseTearDown() override {}

    static constexpr auto fibonacci_expected = []() {
        constexpr size_t n = 95;
        std::array<uint64_t, n> array;
        array[0] = 0, array[1] = 1;
        uint64_t a = 0, b = 1;
        for (size_t i = 2; i < n; ++i) {
            array[i] = a + b;
            a = b;
            b = array[i];
        }
        return array;
    }();

    Generator<uint64_t> fibonacci_sequence(unsigned n) {
        if (n == 0)
            co_return;

        if (n > 94)
            throw std::runtime_error(
                "Too big Fibonacci sequence. Elements would overflow.");

        co_yield 0;

        if (n == 1)
            co_return;

        co_yield 1;

        if (n == 2)
            co_return;

        uint64_t a = 0;
        uint64_t b = 1;

        for (unsigned i = 2; i < n; i++) {
            uint64_t s = a + b;
            co_yield s;
            a = b;
            b = s;

            // a lucky number
            if (i == 51) {
                throw std::logic_error(
                    "Exception thrown in the middle of the test.");
            }
        }
    }

    Generator<int> iota(int start = 0) {
        while (true) {
            co_yield start++;
        }
    }
};

TEST_F(GeneratorTest, testOperator) {
    size_t n = 16;
    auto gen = fibonacci_sequence(n);
    for (int j = 0; gen; ++j) {
        EXPECT_EQ(gen(), fibonacci_expected[j]);
    }
}

TEST_F(GeneratorTest, testIterator) {
    size_t n = 15;
    for (int j = 0; int i : fibonacci_sequence(n)) {
        EXPECT_EQ(i, fibonacci_expected[j++]);
    }
}

TEST_F(GeneratorTest, testGenerate) {
    std::size_t n = 10;
    std::vector<int> vec(n);
    std::generate(vec.begin(), vec.end(), fibonacci_sequence(n));
    for (int j = 0; int i : vec) {
        EXPECT_EQ(i, fibonacci_expected[j++]);
    }
}

// FIXME: clang complile fail
#ifndef __clang__

TEST_F(GeneratorTest, testRange) {
    int j = 0;
    for (auto i : iota() | std::views::take(10)) {
        EXPECT_EQ(i, j++);
    }
    EXPECT_EQ(j, 10);
}
#endif  // __clang__

TEST_F(GeneratorTest, testException) {
    size_t n = 95;
    try {
        auto gen = fibonacci_sequence(n);
        for (int j = 0; gen; ++j) {
            EXPECT_EQ(gen(), fibonacci_expected[j]);
        }
        EXPECT_FALSE(true);
    } catch (const std::runtime_error& ex) {
        EXPECT_STREQ(ex.what(),
                     "Too big Fibonacci sequence. Elements would overflow.");
    } catch (...) {
        EXPECT_FALSE(true);
    }

    try {
        for (int j = 0; auto i : fibonacci_sequence(n)) {
            EXPECT_EQ(i, fibonacci_expected[j++]);
        }
        EXPECT_FALSE(true);
    } catch (const std::runtime_error& ex) {
        EXPECT_STREQ(ex.what(),
                     "Too big Fibonacci sequence. Elements would overflow.");
    } catch (...) {
        EXPECT_FALSE(true);
    }

    try {
        std::vector<int> vec(n);
        std::generate(vec.begin(), vec.end(), fibonacci_sequence(n));
        for (int j = 0; auto i : vec) {
            EXPECT_EQ(i, fibonacci_expected[j++]);
        }
        EXPECT_FALSE(true);
    } catch (const std::runtime_error& ex) {
        EXPECT_STREQ(ex.what(),
                     "Too big Fibonacci sequence. Elements would overflow.");
    } catch (...) {
        EXPECT_FALSE(true);
    }
}

TEST_F(GeneratorTest, testLuckException) {
    size_t n = 55;
    try {
        auto gen = fibonacci_sequence(n);
        for (int j = 0; gen; ++j) {
            EXPECT_EQ(gen(), fibonacci_expected[j]);
        }
        EXPECT_FALSE(true);
    } catch (const std::logic_error& ex) {
        EXPECT_STREQ(ex.what(), "Exception thrown in the middle of the test.");
    } catch (...) {
        EXPECT_FALSE(true);
    }

    try {
        for (int j = 0; auto i : fibonacci_sequence(n)) {
            EXPECT_EQ(i, fibonacci_expected[j++]);
        }
        EXPECT_FALSE(true);
    } catch (const std::logic_error& ex) {
        EXPECT_STREQ(ex.what(), "Exception thrown in the middle of the test.");
    } catch (...) {
        EXPECT_FALSE(true);
    }

    try {
        std::vector<int> vec(n);
        std::generate(vec.begin(), vec.end(), fibonacci_sequence(n));
        for (int j = 0; auto i : vec) {
            EXPECT_EQ(i, fibonacci_expected[j++]);
        }
        EXPECT_FALSE(true);
    } catch (const std::logic_error& ex) {
        EXPECT_STREQ(ex.what(), "Exception thrown in the middle of the test.");
    } catch (...) {
        EXPECT_FALSE(true);
    }
}

TEST_F(GeneratorTest, testNoDefaultConstruct) {
    struct A {
        A(int) {}
        A() = delete;
    };
    auto lambda = []() -> Generator<A> { co_yield 1; };
    for (auto& a : lambda()) {
        (void)a;
    }
    auto gen = lambda();
    while (gen) {
        auto a = gen();
        (void)a;
    }
}

TEST_F(GeneratorTest, testYieldMultiValue) {
    auto lambda = []() -> Generator<std::tuple<int, std::string>> {
        for (int i = 0; i < 10; ++i) {
            co_yield std::tuple{i, std::to_string(i)};
        }
    };

    for (const auto& [i, str_i] : lambda()) {
        EXPECT_EQ(i, std::stoi(str_i));
    }

    auto gen = lambda();
    while (gen) {
        auto [i, str_i] = gen();
        EXPECT_EQ(i, std::stoi(str_i));
    }
}

TEST_F(GeneratorTest, testNoMoveClass) {
    struct A {
        A() = default;
        A(const A&) = default;
        A& operator=(const A&) = default;
        A(A&&) = delete;
        A& operator=(A&&) = delete;
    };

    auto lambda = []() -> Generator<A> {
        A a;
        co_yield a;
    };

    for (auto& a : lambda()) {
        (void)a;
    }

    // Can't move, so can't use.
    // This is exactly the difference between the iterator pattern and
    // `operator()`. The iterator returns T&, but `operator()` returns T

    // auto gen = lambda();
    // while (gen) {
    //     (void)gen();
    // }
}

TEST_F(GeneratorTest, testNoCopyClass) {
    struct A {
        A() = default;
        A(A&&) = default;
        A(const A&) = delete;
        A& operator=(const A&) = delete;
    };

    auto lambda = []() -> Generator<A> { co_yield A{}; };

    for (auto& a : lambda()) {
        (void)a;
    }

    auto gen = lambda();
    while (gen) {
        (void)gen();
    }
}

}  // namespace async_simple::coro
