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
#include <gmock/gmock.h>

using testing::DoAll;
using testing::ElementsAre;
using testing::InSequence;
using testing::Invoke;
using testing::Return;
using testing::ReturnRef;
using testing::SetArgReferee;
using testing::Throw;
using testing::UnorderedElementsAre;

class FUTURE_TESTBASE : public testing::Test {
public:
    virtual void caseSetUp() = 0;
    virtual void caseTearDown() = 0;

    void SetUp() override { caseSetUp(); }

    void TearDown() override { caseTearDown(); }
};