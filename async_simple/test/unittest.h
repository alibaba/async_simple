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
#ifndef __ASYNC_SIMPLE_UNITTEST_H
#define __ASYNC_SIMPLE_UNITTEST_H

#include <string>
#include <typeinfo>
#define GTEST_USE_OWN_TR1_TUPLE 0
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class FUTURE_TESTBASE : public testing::Test {
public:
    virtual void caseSetUp() = 0;
    virtual void caseTearDown() = 0;

    void SetUp() override { caseSetUp(); }

    void TearDown() override { caseTearDown(); }
};

#endif  //__ASYNC_SIMPLE_UNITTEST_H
