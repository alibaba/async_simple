/*
 * Copyright (c) 2024, Alibaba Group Holding Limited;
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
#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include "async_simple/Cancellation.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"

using namespace std;
using namespace async_simple::executors;
using namespace async_simple;

namespace async_simple {

class CancellationTest : public FUTURE_TESTBASE {
public:
    void caseSetUp() override {}
    void caseTearDown() override {}
};
TEST_F(CancellationTest, testSimpleCancellation) {
    auto signal = CancellationSignal::create();
    auto slot = std::make_shared<CancellationSlot>(signal.get());
    auto result = std::async([slot] {
        while (!slot->canceled()) {
            std::this_thread::yield();
        }
        return slot->signal()->state();
    });
    EXPECT_EQ(signal->hasEmited(), false);
    EXPECT_EQ(signal->emit(CancellationType::terminal), true);
    EXPECT_EQ(signal->hasEmited(), true);
    EXPECT_EQ(result.get(), CancellationType::terminal);
    EXPECT_EQ(slot->canceled(), true);
    EXPECT_EQ(slot->signal()->state(), CancellationType::terminal);
    EXPECT_EQ(signal->state(), CancellationType::terminal);
}

TEST_F(CancellationTest, testCancellationWithNoneType) {
    std::atomic<bool> flag = false;
    auto signal = CancellationSignal::create();
    auto slot = std::make_shared<CancellationSlot>(signal.get());
    auto result = std::async([slot, &flag] {
        while (!slot->canceled()) {
            if (flag) {
                return 0;
            }
            std::this_thread::yield();
        }
        return 1;
    });
    signal->emit(CancellationType::none);
    std::this_thread::sleep_for(10ms);
    flag = 1;
    EXPECT_EQ(result.get(), 0);
    EXPECT_EQ(slot->canceled(), false);
    EXPECT_EQ(slot->signal()->state(), CancellationType::none);
    EXPECT_EQ(signal->state(), CancellationType::none);
}

TEST_F(CancellationTest, testCancellationWithSlotFilter) {
    std::atomic<bool> flag = false;
    auto signal = CancellationSignal::create();
    auto slot = std::make_shared<CancellationSlot>(
        signal.get(), static_cast<CancellationType>(1));
    auto result = std::async([slot, &flag] {
        while (!slot->canceled()) {
            if (flag) {
                return 0;
            }
            std::this_thread::yield();
        }
        return 1;
    });
    signal->emit(CancellationType::terminal);
    std::this_thread::sleep_for(10ms);
    flag = 1;
    EXPECT_EQ(result.get(), 0);
    EXPECT_EQ(slot->canceled(), false);
    EXPECT_EQ(slot->signal()->state(), CancellationType::terminal);
    EXPECT_EQ(signal->state(), CancellationType::terminal);
}

TEST_F(CancellationTest, testMultiSlotsCancellationCallback) {
    auto signal = CancellationSignal::create();
    std::vector<std::unique_ptr<CancellationSlot>> slots;
    int j = 0;
    for (int i = 0; i < 100; ++i) {
        slots.emplace_back(std::make_unique<CancellationSlot>(signal.get()));
        [[maybe_unused]] auto _ =
            slots.back()->emplace([&j](CancellationType type) mutable {
                EXPECT_EQ(type, CancellationType::terminal);
                j++;
            });
    }
    signal->emit(CancellationType::terminal);
    EXPECT_EQ(j, 100);
}

TEST_F(CancellationTest, testMultiSlotsCancellation) {
    auto signal = CancellationSignal::create();
    std::vector<std::unique_ptr<CancellationSlot>> slots;
    int j = 0;
    for (int i = 0; i < 100; ++i) {
        slots.emplace_back(std::make_unique<CancellationSlot>(signal.get()));
        [[maybe_unused]] auto _ =
            slots.back()->emplace([&j](CancellationType) mutable { j++; });
    }
    signal->emit(CancellationType::terminal);
    EXPECT_EQ(j, 100);
}

TEST_F(CancellationTest, testMultiSlotsCancellationWithFilter) {
    {
        auto signal = CancellationSignal::create();
        std::vector<std::unique_ptr<CancellationSlot>> slots;
        int j = 0;
        CancellationType expected_type = CancellationType::terminal;
        for (int i = 0; i < 100; ++i) {
            slots.emplace_back(std::make_unique<CancellationSlot>(
                signal.get(), (i % 4) ? (CancellationType::all)
                                      : static_cast<CancellationType>(0b11)));
            [[maybe_unused]] auto _ = slots.back()->emplace(
                [&j, &expected_type](CancellationType type) mutable {
                    j++;
                    EXPECT_EQ(type, expected_type);
                });
        }
        signal->emit(expected_type);
        EXPECT_EQ(j, 75);
        signal->emit(static_cast<CancellationType>(0b10));
        EXPECT_EQ(j, 75);
    }
    {
        auto signal = CancellationSignal::create();
        std::vector<std::unique_ptr<CancellationSlot>> slots;
        int j = 0;
        CancellationType expected_type = static_cast<CancellationType>(3);
        for (int i = 0; i < 100; ++i) {
            slots.emplace_back(std::make_unique<CancellationSlot>(
                signal.get(), (i % 4) ? (CancellationType::all)
                                      : static_cast<CancellationType>(1)));
            [[maybe_unused]] auto _ = slots.back()->emplace(
                [&j, &expected_type](CancellationType type) mutable {
                    j++;
                    EXPECT_EQ(type, expected_type);
                });
        }
        signal->emit(expected_type);
        EXPECT_EQ(j, 100);
    }
}

TEST_F(CancellationTest, testScopeFilterGuard) {
    {
        auto signal = CancellationSignal::create();
        auto slot = std::make_unique<CancellationSlot>(signal.get());
        [[maybe_unused]] auto _ =
            slot->emplace([](CancellationType type) { EXPECT_TRUE(true); });
        signal->emit(CancellationType::terminal);
        {
            auto guard =
                slot->addScopedFilter(static_cast<CancellationType>(1));
            EXPECT_TRUE(guard == false);
            EXPECT_TRUE(slot->canceled());
        }
        EXPECT_TRUE(slot->canceled());
    }
    {
        auto signal = CancellationSignal::create();
        auto slot = std::make_unique<CancellationSlot>(signal.get());
        [[maybe_unused]] auto _ =
            slot->emplace([](CancellationType type) { EXPECT_TRUE(false); });
        {
            auto guard =
                slot->addScopedFilter(static_cast<CancellationType>(1));
            signal->emit(CancellationType::terminal);
            EXPECT_TRUE(guard);
            EXPECT_TRUE(slot->signal()->state() == CancellationType::terminal);
            EXPECT_TRUE(slot->canceled() == false);
        }
        EXPECT_TRUE(slot->canceled());
    }
    {
        auto signal = CancellationSignal::create();
        auto slot = std::make_unique<CancellationSlot>(signal.get());
        [[maybe_unused]] auto _ =
            slot->emplace([](CancellationType type) { EXPECT_TRUE(true); });
        {
            auto guard = slot->addScopedFilter(CancellationType::all);
            signal->emit(CancellationType::terminal);
            EXPECT_TRUE(guard == true);
            EXPECT_TRUE(slot->canceled());
        }
        EXPECT_TRUE(slot->canceled());
    }
}

TEST_F(CancellationTest, testScopeFilterGuardNested) {
    {
        auto signal = CancellationSignal::create();
        auto slot = std::make_unique<CancellationSlot>(signal.get());
        [[maybe_unused]] auto _ =
            slot->emplace([](CancellationType type) { EXPECT_TRUE(true); });
        {
            auto guard =
                slot->addScopedFilter(static_cast<CancellationType>(0b111));
            {
                auto guard = slot->addScopedFilter(
                    static_cast<CancellationType>(0b11100));
                signal->emit(static_cast<CancellationType>(0b100));
                EXPECT_EQ(slot->getFilter(),
                          static_cast<CancellationType>(0b100));
                EXPECT_EQ(slot->canceled(), true);
            }
            EXPECT_EQ(
                slot->addScopedFilter(static_cast<CancellationType>(0b100)),
                false);
            EXPECT_EQ(slot->getFilter(), static_cast<CancellationType>(0b111));
            EXPECT_EQ(slot->canceled(), true);
        }
        EXPECT_EQ(slot->getFilter(), CancellationType::all);
        EXPECT_EQ(slot->canceled(), true);
    }
    {
        auto signal = CancellationSignal::create();
        auto slot = std::make_unique<CancellationSlot>(signal.get());
        [[maybe_unused]] auto _ =
            slot->emplace([](CancellationType type) { EXPECT_TRUE(false); });
        {
            auto guard =
                slot->addScopedFilter(static_cast<CancellationType>(0b111));
            {
                auto guard = slot->addScopedFilter(
                    static_cast<CancellationType>(0b11100));
                signal->emit(static_cast<CancellationType>(0b011));
                EXPECT_EQ(slot->getFilter(),
                          static_cast<CancellationType>(0b100));
                EXPECT_EQ(slot->canceled(), false);
            }
            EXPECT_EQ(
                slot->addScopedFilter(static_cast<CancellationType>(0b100)),
                false);
            EXPECT_EQ(slot->getFilter(), static_cast<CancellationType>(0b111));
            EXPECT_EQ(slot->canceled(), true);
        }
        EXPECT_EQ(slot->getFilter(), CancellationType::all);
        EXPECT_EQ(slot->canceled(), true);
    }
}

TEST_F(CancellationTest, testMultiThreadEmit) {
    for (int i = 0; i < 10; ++i) {
        auto signal = CancellationSignal::create();
        std::vector<std::future<bool>> res;
        std::vector<std::unique_ptr<CancellationSlot>> slots;
        int j = 0;
        std::atomic<bool> start_flag = false;
        for (int i = 0; i < 1000; ++i) {
            slots.emplace_back(
                std::make_unique<CancellationSlot>(signal.get()));
            [[maybe_unused]] bool _ =
                slots.back()->emplace([&j](CancellationType type) { ++j; });
        };
        for (int i = 0; i < 100; ++i)
            res.emplace_back(std::async([&]() {
                while (!start_flag)
                    std::this_thread::yield();
                return signal->emit(CancellationType::terminal);
            }));
        start_flag = true;
        int cnt = 0;
        for (auto& e : res) {
            auto value = e.get();
            if (value) {
                ++cnt;
            }
        };
        EXPECT_EQ(cnt, 1);
        EXPECT_EQ(signal->state(), CancellationType::terminal);
        EXPECT_EQ(j, 1000);
    }
}

TEST_F(CancellationTest, testMultiThreadEmitWhenEmplace) {
    for (int i = 0; i < 10; ++i) {
        auto signal = CancellationSignal::create();
        std::vector<std::future<bool>> res;
        std::vector<std::unique_ptr<CancellationSlot>> slots;
        slots.resize(100);
        int j = 0;
        std::atomic<bool> start_flag = false;
        auto add_slot = [&](int i) {
            res.emplace_back(
                std::async([&slots, i, &j, &start_flag, &signal]() {
                    while (!start_flag)
                        std::this_thread::yield();
                    slots[i] = std::make_unique<CancellationSlot>(signal.get());
                    bool ok =
                        slots[i]->emplace([&j](CancellationType type) { ++j; });
                    if (!ok) {
                        EXPECT_TRUE(slots[i]->canceled());
                        EXPECT_FALSE(slots[i]->hasStartExecute());
                    }
                    return false;
                }));
        };
        start_flag = true;
        add_slot(0);
        res[0].wait();
        start_flag = false;
        for (int i = 1; i < 100; ++i) {
            add_slot(i);
        };
        for (int i = 0; i < 100; ++i)
            res.emplace_back(std::async([&]() {
                while (!start_flag)
                    std::this_thread::yield();
                return signal->emit(CancellationType::terminal);
            }));
        start_flag = true;
        int cnt = 0;
        for (auto& e : res) {
            auto value = e.get();
            if (value) {
                ++cnt;
            }
        };
        EXPECT_EQ(cnt, 1);
        EXPECT_EQ(signal->state(), CancellationType::terminal);
        std::cout << "trigger slot cnt: " << j << std::endl;
        EXPECT_GT(j, 0);
        EXPECT_LE(j, 100);
    }
}
TEST_F(CancellationTest, testMultiThreadEmitWhenAddScopedFilter) {
    int cnter = 0, cnter2 = 0;
    std::atomic<bool> start_flag;
    std::atomic<int> start_flag2 = false;
    std::vector<std::thread> works;
    for (int i = 0; i < 100; ++i) {
        auto signal = CancellationSignal::create();
        auto slot = std::make_shared<CancellationSlot>(signal.get());
        works.emplace_back([slot, &cnter, &cnter2, &start_flag,
                            &start_flag2]() {
            auto ok =
                slot->emplace([&cnter2](CancellationType type) { ++cnter2; });
            EXPECT_TRUE(ok);
            start_flag2++;
            while (!start_flag) {
                std::this_thread::yield();
            }
            auto guard = slot->addScopedFilter(CancellationType::none);
            ok = slot->emplace([&cnter](CancellationType type) { ++cnter; });
            if (!ok) {
                EXPECT_TRUE(slot->hasStartExecute());
            }
            slot->clear();
        });
        works.emplace_back([signal, &start_flag, &start_flag2]() {
            start_flag2++;
            while (!start_flag) {
                std::this_thread::yield();
            }
            signal->emit(CancellationType::terminal);
        });
    }
    while (start_flag2 < 200) {
        std::this_thread::yield();
    }
    start_flag = true;
    for (auto& e : works) {
        e.join();
    }
    EXPECT_EQ(cnter, 0);
    EXPECT_GE(cnter2, 0);
    EXPECT_LE(cnter2, 100);
    std::cout << "cnter2:" << cnter2 << std::endl;
}
TEST_F(CancellationTest, testRegistSignalAfterCancellation) {
    {
        auto signal = CancellationSignal::create();
        signal->emit(CancellationType::terminal);
        CancellationSlot s{signal.get()};
        EXPECT_FALSE(s.emplace([](CancellationType) {}));
        EXPECT_FALSE(s.clear());
        EXPECT_FALSE(s.clear());
        auto guard = s.addScopedFilter(CancellationType::terminal);
        EXPECT_FALSE(guard);
        EXPECT_EQ(s.getFilter(), CancellationType::all);
    }
}
}  // namespace async_simple