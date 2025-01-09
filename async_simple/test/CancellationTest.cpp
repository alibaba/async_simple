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
#include "async_simple/Signal.h"
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
    auto signal = Signal::create();
    auto slot = std::make_shared<Slot>(signal.get());
    auto result = std::async([slot] {
        while (!slot->canceled()) {
            std::this_thread::yield();
        }
        return slot->signal()->state();
    });
    EXPECT_EQ(signal->state() != 0, false);
    EXPECT_EQ(signal->emit(SignalType::terminate), true);
    EXPECT_EQ(signal->state() != 0, true);
    EXPECT_EQ(result.get(), SignalType::terminate);
    EXPECT_EQ(slot->canceled(), true);
    EXPECT_EQ(slot->signal()->state(), SignalType::terminate);
    EXPECT_EQ(signal->state(), SignalType::terminate);
}

TEST_F(CancellationTest, testCancellationWithNoneType) {
    std::atomic<bool> flag = false;
    auto signal = Signal::create();
    auto slot = std::make_shared<Slot>(signal.get());
    auto result = std::async([slot, &flag] {
        while (!slot->canceled()) {
            if (flag) {
                return 0;
            }
            std::this_thread::yield();
        }
        return 1;
    });
    signal->emit(SignalType::none);
    std::this_thread::sleep_for(10ms);
    flag = 1;
    EXPECT_EQ(result.get(), 0);
    EXPECT_EQ(slot->canceled(), false);
    EXPECT_EQ(slot->signal()->state(), SignalType::none);
    EXPECT_EQ(signal->state(), SignalType::none);
}

TEST_F(CancellationTest, testCancellationWithSlotFilter) {
    std::atomic<bool> flag = false;
    auto signal = Signal::create();
    auto slot =
        std::make_shared<Slot>(signal.get(), static_cast<SignalType>(0b10));
    auto result = std::async([slot, &flag] {
        while (!slot->canceled()) {
            if (flag) {
                return 0;
            }
            std::this_thread::yield();
        }
        return 1;
    });
    signal->emit(SignalType::terminate);
    std::this_thread::sleep_for(10ms);
    flag = 1;
    EXPECT_EQ(result.get(), 0);
    EXPECT_EQ(slot->canceled(), false);
    EXPECT_EQ(slot->signal()->state(), SignalType::terminate);
    EXPECT_EQ(signal->state(), SignalType::terminate);
}

TEST_F(CancellationTest, testMultiSlotsCancellationCallback) {
    auto signal = Signal::create();
    std::vector<std::unique_ptr<Slot>> slots;
    int j = 0;
    for (int i = 0; i < 100; ++i) {
        slots.emplace_back(std::make_unique<Slot>(signal.get()));
        [[maybe_unused]] auto _ = slots.back()->emplace(
            SignalType::terminate, [&j](SignalType type, Signal*) mutable {
                EXPECT_EQ(type, SignalType::terminate);
                j++;
            });
    }
    signal->emit(SignalType::terminate);
    EXPECT_EQ(j, 100);
}

TEST_F(CancellationTest, testMultiSlotsCancellation) {
    auto signal = Signal::create();
    std::vector<std::unique_ptr<Slot>> slots;
    int j = 0;
    for (int i = 0; i < 100; ++i) {
        slots.emplace_back(std::make_unique<Slot>(signal.get()));
        [[maybe_unused]] auto _ = slots.back()->emplace(
            SignalType::terminate, [&j](SignalType, Signal*) mutable { j++; });
    }
    signal->emit(SignalType::terminate);
    EXPECT_EQ(j, 100);
}

TEST_F(CancellationTest, testMultiSlotsCancellationWithFilter) {
    {
        auto signal = Signal::create();
        std::vector<std::unique_ptr<Slot>> slots;
        int j = 0;
        SignalType expected_type = SignalType::terminate;
        for (int i = 0; i < 100; ++i) {
            slots.emplace_back(std::make_unique<Slot>(
                signal.get(),
                (i % 4) ? (SignalType::all) : static_cast<SignalType>(0b110)));
            [[maybe_unused]] auto _ = slots.back()->emplace(
                SignalType::terminate,
                [&j, &expected_type](SignalType type, Signal*) mutable {
                    j++;
                    EXPECT_EQ(type, expected_type);
                });
        }
        signal->emit(expected_type);
        EXPECT_EQ(j, 75);
        signal->emit(static_cast<SignalType>(0b100));
        EXPECT_EQ(j, 75);
    }
    {
        auto signal = Signal::create();
        std::vector<std::unique_ptr<Slot>> slots;
        int j = 0;
        SignalType expected_type = static_cast<SignalType>(0b11);
        for (int i = 0; i < 100; ++i) {
            auto filter =
                (i % 4) ? (SignalType::all) : static_cast<SignalType>(0b1);
            slots.emplace_back(std::make_unique<Slot>(signal.get(), filter));
            [[maybe_unused]] auto _ = slots.back()->emplace(
                static_cast<SignalType>(0b1),
                [&j, &expected_type, filter](SignalType type, Signal*) mutable {
                    j++;
                    EXPECT_EQ(type, expected_type & filter);
                });
        }
        signal->emit(expected_type);
        EXPECT_EQ(j, 100);
    }
}

TEST_F(CancellationTest, testScopeFilterGuard) {
    {
        auto signal = Signal::create();
        auto slot = std::make_unique<Slot>(signal.get());
        [[maybe_unused]] auto _ =
            slot->emplace(SignalType::terminate,
                          [](SignalType type, Signal*) { EXPECT_TRUE(true); });
        signal->emit(SignalType::terminate);
        {
            auto guard = slot->setScopedFilter(static_cast<SignalType>(1));
            EXPECT_TRUE(slot->canceled());
        }
        EXPECT_TRUE(slot->canceled());
    }
    {
        auto signal = Signal::create();
        auto slot = std::make_unique<Slot>(signal.get());
        [[maybe_unused]] auto _ =
            slot->emplace(SignalType::terminate,
                          [](SignalType type, Signal*) { EXPECT_TRUE(false); });
        {
            auto guard = slot->setScopedFilter(static_cast<SignalType>(0b10));
            signal->emit(SignalType::terminate);
            EXPECT_TRUE(slot->signal()->state() == SignalType::terminate);
            EXPECT_TRUE(slot->canceled() == false);
        }
        EXPECT_TRUE(slot->canceled());
    }
    {
        auto signal = Signal::create();
        auto slot = std::make_unique<Slot>(signal.get());
        [[maybe_unused]] auto _ =
            slot->emplace(SignalType::terminate,
                          [](SignalType type, Signal*) { EXPECT_TRUE(true); });
        {
            auto guard = slot->setScopedFilter(SignalType::all);
            signal->emit(SignalType::terminate);
            EXPECT_TRUE(slot->canceled());
        }
        EXPECT_TRUE(slot->canceled());
    }
}

TEST_F(CancellationTest, testScopeFilterGuardNested) {
    {
        auto signal = Signal::create();
        auto slot = std::make_unique<Slot>(signal.get());
        [[maybe_unused]] auto _ =
            slot->emplace(SignalType::terminate,
                          [](SignalType type, Signal*) { EXPECT_TRUE(true); });
        {
            auto guard = slot->setScopedFilter(static_cast<SignalType>(0b111));
            {
                auto guard =
                    slot->setScopedFilter(static_cast<SignalType>(0b11100));
                signal->emit(static_cast<SignalType>(0b100));
                EXPECT_EQ(slot->getFilter(), static_cast<SignalType>(0b100));
                EXPECT_EQ(slot->hasTriggered(static_cast<SignalType>(0b100)),
                          true);
            }
            EXPECT_EQ(slot->getFilter(), static_cast<SignalType>(0b111));
            EXPECT_EQ(slot->hasTriggered(static_cast<SignalType>(0b100)), true);
        }
        EXPECT_EQ(slot->getFilter(), SignalType::all);
        EXPECT_EQ(slot->hasTriggered(static_cast<SignalType>(0b100)), true);
    }
    {
        auto signal = Signal::create();
        auto slot = std::make_unique<Slot>(signal.get());
        [[maybe_unused]] auto _ =
            slot->emplace(SignalType::terminate,
                          [](SignalType type, Signal*) { EXPECT_TRUE(false); });
        {
            auto guard = slot->setScopedFilter(static_cast<SignalType>(0b111));
            {
                auto guard =
                    slot->setScopedFilter(static_cast<SignalType>(0b11100));
                signal->emit(static_cast<SignalType>(0b011));
                EXPECT_EQ(slot->getFilter(), static_cast<SignalType>(0b100));
                EXPECT_EQ(slot->canceled(), false);
            }
            EXPECT_EQ(slot->getFilter(), static_cast<SignalType>(0b111));
            EXPECT_EQ(slot->canceled(), true);
        }
        EXPECT_EQ(slot->getFilter(), SignalType::all);
        EXPECT_EQ(slot->canceled(), true);
    }
}

TEST_F(CancellationTest, testMultiThreadEmit) {
    for (int i = 0; i < 10; ++i) {
        auto signal = Signal::create();
        std::vector<std::future<SignalType>> res;
        std::vector<std::unique_ptr<Slot>> slots;
        int j = 0;
        std::atomic<bool> start_flag = false;
        for (int i = 0; i < 1000; ++i) {
            slots.emplace_back(std::make_unique<Slot>(signal.get()));
            [[maybe_unused]] bool _ = slots.back()->emplace(
                SignalType::terminate, [&j](SignalType type, Signal*) { ++j; });
        };
        for (int i = 0; i < 100; ++i)
            res.emplace_back(std::async([&]() {
                while (!start_flag)
                    std::this_thread::yield();
                return signal->emit(SignalType::terminate);
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
        EXPECT_EQ(signal->state(), SignalType::terminate);
        EXPECT_EQ(j, 1000);
    }
}

TEST_F(CancellationTest, testMultiThreadEmitDifferentSignal) {
    for (int i = 0; i < 10; ++i) {
        auto signal = Signal::create();
        std::vector<std::future<SignalType>> res;
        std::vector<std::unique_ptr<Slot>> slots;
        std::atomic<int> j[4];
        std::atomic<bool> start_flag = false;
        for (int i = 0; i < 1000; ++i) {
            slots.emplace_back(std::make_unique<Slot>(signal.get()));
            [[maybe_unused]] bool _ = slots.back()->emplace(
                SignalType::terminate,
                [&j](SignalType type, Signal*) { ++j[0]; });
            _ = slots.back()->emplace(
                static_cast<SignalType>(0b10),
                [&j](SignalType type, Signal*) { ++j[1]; });
            _ = slots.back()->emplace(
                static_cast<SignalType>(0b100),
                [&j](SignalType type, Signal*) { ++j[2]; });
            _ = slots.back()->emplace(
                static_cast<SignalType>(uint64_t{1} << 63),
                [&j](SignalType type, Signal*) { ++j[3]; });
        };
        for (int i = 0; i < 100; ++i)
            res.emplace_back(std::async([i, &start_flag, &signal]() {
                while (!start_flag)
                    std::this_thread::yield();
                return signal->emit(
                    i % 2
                        ? static_cast<SignalType>(0b101 | (uint64_t{1} << 63))
                        : static_cast<SignalType>(0b110 | (uint64_t{1} << 63)));
            }));
        start_flag = true;
        int cnt = 0, cnt2 = 0;
        for (auto& e : res) {
            auto value = e.get();
            EXPECT_TRUE(value & (uint64_t{1} << 63));
            uint64_t v = value;
            v ^= (uint64_t{1} << 63);
            if (v) {
                cnt += std::popcount(v);
                cnt2 += 1;
            }
        };
        EXPECT_EQ(cnt, 3);
        EXPECT_EQ(cnt2, 2);
        EXPECT_EQ(signal->state(),
                  static_cast<SignalType>(0b111 | (uint64_t{1} << 63)));
        EXPECT_EQ(j[0], 1000);
        EXPECT_EQ(j[1], 1000);
        EXPECT_EQ(j[2], 1000);
        EXPECT_EQ(j[3], 100000);
    }
}

TEST_F(CancellationTest, testMultiThreadEmitWhenEmplace) {
    for (int i = 0; i < 10; ++i) {
        auto signal = Signal::create();
        std::vector<std::future<bool>> res;
        std::vector<std::unique_ptr<Slot>> slots;
        slots.resize(100);
        int j = 0;
        std::atomic<bool> start_flag = false;
        auto add_slot = [&](int i) {
            res.emplace_back(
                std::async([&slots, i, &j, &start_flag, &signal]() {
                    while (!start_flag)
                        std::this_thread::yield();
                    slots[i] = std::make_unique<Slot>(signal.get());
                    bool ok = slots[i]->emplace(
                        SignalType::terminate,
                        [&j](SignalType type, Signal*) { ++j; });
                    if (!ok) {
                        EXPECT_TRUE(slots[i]->canceled());
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
                return static_cast<bool>(signal->emit(SignalType::terminate));
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
        EXPECT_EQ(signal->state(), SignalType::terminate);
        std::cout << "trigger slot cnt: " << j << std::endl;
        EXPECT_GT(j, 0);
        EXPECT_LE(j, 100);
    }
}
TEST_F(CancellationTest, testMultiThreadEmitWhenSetScopedFilter) {
    int cnter = 0, cnter2 = 0;
    std::atomic<bool> start_flag;
    std::atomic<int> start_flag2 = false;
    std::vector<std::thread> works;
    for (int i = 0; i < 100; ++i) {
        auto signal = Signal::create();
        auto slot = std::make_shared<Slot>(signal.get());
        works.emplace_back([slot, &cnter, &cnter2, &start_flag,
                            &start_flag2]() {
            auto ok = slot->emplace(
                SignalType::terminate,
                [&cnter2](SignalType type, Signal*) { ++cnter2; });
            EXPECT_TRUE(ok);
            start_flag2++;
            while (!start_flag) {
                std::this_thread::yield();
            }
            auto guard = slot->setScopedFilter(SignalType::none);
            ok = slot->emplace(SignalType::terminate,
                               [&cnter](SignalType type, Signal*) { ++cnter; });
            slot->clear(SignalType::terminate);
        });
        works.emplace_back([signal, &start_flag, &start_flag2]() {
            start_flag2++;
            while (!start_flag) {
                std::this_thread::yield();
            }
            signal->emit(SignalType::terminate);
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
        auto signal = Signal::create();
        signal->emit(SignalType::terminate);
        Slot s{signal.get()};
        EXPECT_FALSE(
            s.emplace(SignalType::terminate, [](SignalType, Signal*) {}));
        EXPECT_FALSE(s.clear(SignalType::terminate));
        EXPECT_FALSE(s.clear(SignalType::terminate));
        auto guard = s.setScopedFilter(SignalType::terminate);
        EXPECT_EQ(s.getFilter(), SignalType::terminate);
    }
}

TEST_F(CancellationTest, testDerivedSignal) {
    class MySignal : public Signal {
        using Signal::Signal;

    public:
        std::atomic<size_t> myState;
    };

    auto mySignal = Signal::create<MySignal>();
    auto slot = std::make_unique<Slot>(mySignal.get());
    [[maybe_unused]] auto _ = slot->emplace(
        SignalType::terminate, [](SignalType type, Signal* signal) {
            auto mySignal = dynamic_cast<MySignal*>(signal);
            EXPECT_NE(mySignal, nullptr);
            EXPECT_EQ(mySignal->myState, 1);
        });
    mySignal->myState = 1;
    mySignal->emit(SignalType::terminate);
}
}  // namespace async_simple