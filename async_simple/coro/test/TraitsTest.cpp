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
#include <type_traits>
#include "async_simple/coro/Traits.h"
#include "async_simple/test/unittest.h"

using namespace std;

namespace async_simple {

class Executor;
namespace coro {

class TraitsTest : public FUTURE_TESTBASE {
public:
    void caseSetUp() override {}
    void caseTearDown() override {}
};

class A {
public:
    bool coAwait(Executor *ex) { return true; }
};
class B {
public:
    int value = 0;
};

struct SimpleAwaiter {
    string name;
    SimpleAwaiter(string n) : name(n) {}
};

class C {
public:
    auto operator co_await() { return SimpleAwaiter("C Member"); }
};

auto operator co_await(class A) { return SimpleAwaiter("A Global"); }

TEST_F(TraitsTest, testHasCoAwaitMethod) {
    EXPECT_TRUE(detail::HasCoAwaitMethod<A>);
    EXPECT_FALSE(detail::HasCoAwaitMethod<B>);
}

TEST_F(TraitsTest, testHasCoAwaitOperator) {
    EXPECT_TRUE(detail::HasCoAwaitMethod<A>);
    EXPECT_FALSE(detail::HasMemberCoAwaitOperator<A>);
    EXPECT_TRUE(detail::HasGlobalCoAwaitOperator<A>);
    A a;
    auto awaiterA = detail::getAwaiter(a);
    EXPECT_EQ(string("A Global"), awaiterA.name);

    EXPECT_FALSE(detail::HasCoAwaitMethod<B>);
    EXPECT_FALSE(detail::HasMemberCoAwaitOperator<B>);
    EXPECT_FALSE(detail::HasGlobalCoAwaitOperator<B>);
    B b;
    b.value = 3;
    B awaiterB = detail::getAwaiter(b);
    EXPECT_EQ(3, awaiterB.value);

    EXPECT_FALSE(detail::HasCoAwaitMethod<C>);
    EXPECT_TRUE(detail::HasMemberCoAwaitOperator<C>);
    EXPECT_FALSE(detail::HasGlobalCoAwaitOperator<C>);
    C c;
    auto awaiterC = detail::getAwaiter(c);
    EXPECT_EQ(string("C Member"), awaiterC.name);
}

}  // namespace coro
}  // namespace async_simple
