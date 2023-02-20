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

    // These examples show different usages of reference/value_type
    // template parameters

    // value_type = std::unique_ptr<int>
    // reference = std::unique_ptr<int>&&
    Generator<std::unique_ptr<int>&&> unique_ints(const int high) {
        for (auto i = 0; i < high; ++i) {
            co_yield std::make_unique<int>(i);
        }
    }

    // clang-format off
    // value_type = std::string_view
    // reference = std::string_view
    Generator<std::string_view, std::string_view> string_views() {
        co_yield "foo";
        co_yield "bar";
    }

    Generator<std::string_view, std::string> strings() {
        co_yield "start";
        std::string s;
        for (auto sv : string_views()) {
            s = sv;
            s.push_back('!');
            co_yield s;
        }
        co_yield "end";
    }
    // clang-format on

    struct TreeNode {
        int value;
        TreeNode* left;
        TreeNode* right;
        TreeNode() : value(0), left(nullptr), right(nullptr) {}
        TreeNode(int x) : value(x), left(nullptr), right(nullptr) {}
        TreeNode(int x, TreeNode* left, TreeNode* right)
            : value(x), left(left), right(right) {}
    };

    // Recursive generator

    Generator<int> visit(TreeNode& tree) {
        if (tree.left)
            co_yield ranges::elements_of{visit(*tree.left)};
        co_yield tree.value;
        if (tree.right)
            co_yield ranges::elements_of{visit(*tree.right)};
    }

    Generator<int> slow_visit(TreeNode& tree) {
        if (tree.left) {
            for (int x : slow_visit(*tree.left))
                co_yield x;
        }
        co_yield tree.value;
        if (tree.right) {
            for (int x : slow_visit(*tree.right))
                co_yield x;
        }
    }

    Generator<const std::string&> delete_rows(std::string table,
                                              std::vector<int> ids) {
        for (int id : ids) {
            // co_yield std::format("DELETE FROM {0} WHERE id = {1};", table,
            // id);
            co_yield "DELETE FROM " + table +
                " WHERE id = " + std::to_string(id) + ";";
        }
    }

    Generator<const std::string&> all_queries() {
        std::vector<int> ids1{4, 5, 6, 7};
        std::vector<int> ids2{11, 19};
        co_yield ranges::elements_of{delete_rows("user", ids1)};
        co_yield ranges::elements_of{delete_rows("order", ids2)};
        co_return;
    }

    template <typename T>
    struct stateful_allocator {
        using value_type = T;

        int id;

        explicit stateful_allocator(int id) noexcept : id(id) {}

        template <typename U>
        stateful_allocator(const stateful_allocator<U>& x) : id(x.id) {}

        T* allocate(std::size_t count) {
            std::printf("stateful_allocator(%i).allocate(%zu)\n", id, count);
            return std::allocator<T>().allocate(count);
        }

        void deallocate(T* ptr, std::size_t count) noexcept {
            std::printf("stateful_allocator(%i).deallocate(%zu)\n", id, count);
            std::allocator<T>().deallocate(ptr, count);
        }

        template <typename U>
        bool operator==(const stateful_allocator<U>& x) const {
            return this->id == x.id;
        }
    };

    Generator<int, void, std::allocator<std::byte>> stateless_example() {
        co_yield 42;
    }

    Generator<int, void, std::allocator<std::byte>> stateless_example(
        std::allocator_arg_t, std::allocator<std::byte>) {
        co_yield 42;
    }

    Generator<int> stateless_void_example(std::allocator_arg_t,
                                          std::allocator<std::byte>) {
        co_yield 42;
    }

    template <typename Allocator>
    Generator<int, void, Allocator> stateful_alloc_example(std::allocator_arg_t,
                                                           Allocator) {
        co_yield 42;
    }

    template <typename Allocator>
    Generator<int> stateful_alloc_void_example(std::allocator_arg_t,
                                               Allocator) {
        co_yield 42;
    }
};

TEST_F(GeneratorTest, testIterator) {
    size_t n = 15;
    for (int j = 0; int i : fibonacci_sequence(n)) {
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

TEST_F(GeneratorTest, testExample) {
    int j = 0;
    for (auto&& i : unique_ints(15)) {
        EXPECT_EQ(*i, j++);
    }
    EXPECT_EQ(j, 15);

    for (auto sv : strings()) {
        std::cout << sv << std::endl;
    }

    auto f = []() -> Generator<int, int> {
        std::vector<int> array{9, 2, 0};
        co_yield ranges::elements_of{array};
    };

    std::array expect = {9, 2, 0};
    for (int k = 0; int i : f()) {
        EXPECT_EQ(expect[k++], i);
    }

    for (auto&& str : all_queries()) {
        std::cout << str << std::endl;
    }

    std::printf("\nstateless_alloc examples\n");

    stateless_example();
    stateless_example(std::allocator_arg, std::allocator<float>{});
    stateless_void_example(std::allocator_arg, std::allocator<float>{});

    std::printf("\nstateful_alloc example\n");

    stateful_alloc_example(std::allocator_arg, stateful_allocator<double>{42});
    stateful_alloc_void_example(std::allocator_arg,
                                stateful_allocator<double>{42});
}

TEST_F(GeneratorTest, testRecursion) {
    /*
                [7]
               /  \
             [3] [15]
                 / \
              [9] [20]
    */
    TreeNode node_3(9), node_4(20);
    TreeNode node_1(3), node_2(15, &node_3, &node_4);
    TreeNode root(7, &node_1, &node_2);

    std::vector<int> vec1;
    std::vector<int> vec2;

    for (int i : slow_visit(root)) {
        vec1.push_back(i);
    }
    EXPECT_TRUE(std::is_sorted(vec1.begin(), vec1.end()));

    for (int i : visit(root)) {
        vec2.push_back(i);
    }
    EXPECT_TRUE(std::is_sorted(vec2.begin(), vec2.end()));

    EXPECT_EQ(vec1, vec2);
}

TEST_F(GeneratorTest, testException) {
    size_t n = 95;

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
}

TEST_F(GeneratorTest, testLuckException) {
    size_t n = 55;

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
}

TEST_F(GeneratorTest, testNoDefaultConstruct) {
    struct A {
        A(int) {}
        A() = delete;
    };
    auto lambda = []() -> Generator<A> { co_yield 1; };
    for (auto&& a : lambda()) {
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

    for (auto&& a : lambda()) {
        (void)a;
    }
}

TEST_F(GeneratorTest, testNoCopyClass) {
    struct A {
        A() = default;
        A(A&&) = default;
        A(const A&) = delete;
        A& operator=(const A&) = delete;
    };

    auto lambda = []() -> Generator<A> { co_yield A{}; };

    for (auto&& a : lambda()) {
        (void)a;
    }
}

}  // namespace async_simple::coro
