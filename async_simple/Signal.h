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
#ifndef ASYNC_SIMPLE_SIGNAL_H
#define ASYNC_SIMPLE_SIGNAL_H

#ifndef ASYNC_SIMPLE_USE_MODULES

#include <assert.h>
#include <any>
#include <atomic>
#include <bit>
#include <cstdint>
#include <memory>
#include <system_error>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "async_simple/Common.h"
#include "util/move_only_function.h"
#endif  // ASYNC_SIMPLE_USE_MODULES

namespace async_simple {

enum SignalType : uint64_t {
    none = 0,
    // low 32 bit signal only trigger once.
    // regist those signal's handler after triggered will failed and return
    // false.
    terminate = 1,  // signal type: terminate
    // 1-16 bits reserve for async-simple
    // 17-32 bits could used for user-defined signal

    // high 32 bit signal, could trigger multiple times, emplace an handler
    // 33-48 bits reserve for async-simple
    // 49-64 bits could used for user-defined signal
    // after signal triggered will always success and return true.
    all = UINT64_MAX,
};

class Slot;

class Signal : public std::enable_shared_from_this<Signal> {
private:
    using Handler = util::move_only_function<void(SignalType, Signal*)>;
    using TransferHandler = util::move_only_function<void(SignalType)>;
    struct HandlerManager {
        uint8_t _index;
        std::atomic<Handler*> _handler;
        HandlerManager* _next;
        HandlerManager(uint8_t index, HandlerManager* next)
            : _index(index), _next(next) {}
        void releaseNodes() {
            for (HandlerManager* tmp = _next; tmp != nullptr; tmp = _next) {
                _next = _next->_next;
                delete tmp;
            }
            delete this;
        }
        ~HandlerManager();
        static inline Handler emittedTag;
    };
    struct PrivateConstructTag {};
    struct Node {
        Node(SignalType filter)
            : _handlerManager(), _filter(filter), _next(nullptr) {}
        ~Node();
        void releaseNodes() {
            for (Node *node = this->_next, *next_node; node != nullptr;
                 node = next_node) {
                next_node = node->_next;
                delete node;
            }
            delete this;
        }
        bool triggered(SignalType state) const noexcept {
            return static_cast<uint64_t>(state) &
                   static_cast<uint64_t>(
                       _filter.load(std::memory_order_acquire));
        }
        void setFilter(SignalType filter) noexcept {
            _filter.store(filter, std::memory_order_release);
        }
        SignalType getFilter() const noexcept {
            return _filter.load(std::memory_order_acquire);
        }
        void invoke(SignalType type, SignalType signal, Signal* self,
                    std::atomic<Handler*>& handlerPtr);
        void operator()(SignalType type, Signal* self);
        std::atomic<HandlerManager*> _handlerManager;
        std::atomic<TransferHandler*> _transferHandler;
        std::atomic<SignalType> _filter;
        Node* _next;
    };
    void registSlot(Slot* slot, SignalType filter);

    inline static bool isMultiTriggerSignal(SignalType type) {
        return type >= (1ull << 32);
    }

    inline SignalType UpdateState(std::atomic<uint64_t>& state,
                                  SignalType type) {
        uint64_t expected = state.load(std::memory_order_acquire), validSignal;
        do {
            validSignal = (expected | type) ^ expected;
        } while (validSignal &&
                 !state.compare_exchange_weak(expected, expected | type,
                                              std::memory_order_release));
        return static_cast<SignalType>(validSignal |
                                       (type & (uint64_t{UINT32_MAX} << 32)));
    }

public:
    Signal(PrivateConstructTag){};
    // Same type Signal can only trigger once. This function emit a signal to
    // binding slots, then execute the slot callback functions. It will return
    // the signal which success triggered. If no signal success triggger, return
    // SignalType::none.
    SignalType emit(SignalType state) noexcept;
    friend class Slot;
    // Return cancellation type now.
    SignalType state() const noexcept {
        return static_cast<SignalType>(_state.load(std::memory_order_acquire));
    }
    // Create CancellationSignal by Factory function.
    //
    template <typename T = Signal>
    static std::shared_ptr<T> create() {
        static_assert(std::is_base_of_v<Signal, T>,
                      "T should be signal or derived from Signal");
        return std::make_shared<T>(PrivateConstructTag{});
    }
    virtual ~Signal();

private:
    std::atomic<uint64_t> _state;
    std::atomic<Node*> _slotsHead;
};

class Slot {
public:
    // regist a slot to signal, if signal isn't emitted now, the slot will be
    // connected to signal.
    Slot(Signal* signal, SignalType filter = SignalType::all)
        : _signal(signal->shared_from_this()) {
        signal->registSlot(this, filter);
    };
    Slot(const Slot&) = delete;

public:
    // Register a signal handler. Returns false if the cancellation signal has
    // already been triggered(and signal can't trigger twice), or the slot is
    // disconnected.
    template <typename... Args>
    [[nodiscard]] bool emplace(SignalType type, Args&&... args) {
        static_assert(sizeof...(args) >= 1,
                      "we dont allow emplace an empty signal handler");
        logicAssert(std::popcount(static_cast<uint64_t>(type)) == 1,
                    "It's not allow to emplace for multiple signals");
        // signal has triggered and cant trigger twice
        if (type < (1ull << 32) && (signal()->state() & type)) {
            return false;
        }
        auto handler =
            std::make_unique<Signal::Handler>(std::forward<Args>(args)...);
        logicAssert(static_cast<bool>(*handler),
                    "CancellationSlot dont allow emplace empty handler");
        auto oldHandlerPtr = loadHandler<true>(type);
        auto oldHandler = oldHandlerPtr->load(std::memory_order_acquire);
        if (oldHandler == &Signal::HandlerManager::emittedTag) {
            return false;
        }
        auto new_handler = handler.release();
        if (!oldHandlerPtr->compare_exchange_strong(
                oldHandler, new_handler, std::memory_order_release)) {
            // if slot is canceled and new emplace handler doesn't exec, return
            // false
            assert(oldHandler == &Signal::HandlerManager::emittedTag);
            delete new_handler;
            return false;
        }
        delete oldHandler;
        return true;
    }
    // Clear the signal handler. If return false, it indicates
    // that the signal handler has been executed, the slot is disconnected, or
    // handler is empty
    bool clear(SignalType type) {
        logicAssert(std::popcount(static_cast<uint64_t>(type)) == 1,
                    "It's not allow to emplace for multiple signals");
        auto oldHandlerPtr = loadHandler<false>(type);
        if (oldHandlerPtr == nullptr) {
            return false;
        }
        auto oldHandler = oldHandlerPtr->load(std::memory_order_acquire);
        if (oldHandler == &Signal::HandlerManager::emittedTag ||
            oldHandler == nullptr) {
            return false;
        }
        if (!oldHandlerPtr->compare_exchange_strong(
                oldHandler, nullptr, std::memory_order_release)) {
            assert(oldHandler == &Signal::HandlerManager::emittedTag);
            return false;
        }
        delete oldHandler;
        return true;
    }

    class FilterGuard {
    public:
        ~FilterGuard() noexcept { _slot->_node->setFilter(_oldFilter); }

    private:
        FilterGuard(Slot* slot, SignalType newFilter) noexcept
            : _slot(slot), _oldFilter(_slot->getFilter()) {
            auto filter = static_cast<uint64_t>(_oldFilter) &
                          static_cast<uint64_t>(newFilter);
            _slot->_node->setFilter(static_cast<SignalType>(filter));
        }
        friend class Slot;
        Slot* _slot;
        SignalType _oldFilter;
    };

    // Filter signals within the specified scope. If signal type & filter is 0,
    // then the signal type will not be triggered within this scope. Nested
    // filters are allowed.
    // The returned guard object can be used to restore the previous filter.
    // It's life-time must shorter than Slot.
    [[nodiscard]] FilterGuard setScopedFilter(SignalType filter) noexcept {
        return FilterGuard{this, filter};
    }
    // Set the current scope's filter.
    void setFilter(SignalType filter) noexcept { _node->setFilter(filter); }

    // Get the current scope's filter.
    // if slot is not connect, always return SignalType::all
    SignalType getFilter() const noexcept { return _node->getFilter(); }

    // Check whether the filtered cancellation signal has emitted.
    // if return true, the signal has emitted(but handler may hasn't execute
    // now) if slot is not connect to signal, always return true;
    bool hasTriggered(SignalType type) const noexcept {
        return _node->triggered(
            static_cast<SignalType>(signal()->state() & type));
    }
    bool canceled() const noexcept {
        return hasTriggered(SignalType::terminate);
    }

    // The slot holds ownership of the corresponding signal, so the signal's
    // lifetime is always no shorter than the slot's. To extend the signal's
    // lifetime, you can call signal()->shared_from_this(), or start a new
    // coroutine with the signal.
    // return nullptr when signal isn't connect to slot.
    Signal* signal() const noexcept { return _signal.get(); }

    friend class Signal;

    // bind a sub signal to this slot. allow triggered signal will transfer to
    // this subSignal.
    void addSubSignal(Signal* subSignal) {
        if (subSignal == nullptr) {
            _node->_transferHandler = nullptr;
            return;
        }

        _node->_transferHandler.store(
            new Signal::TransferHandler(
                [subSignal = subSignal->weak_from_this()](SignalType type) {
                    if (auto signal = subSignal.lock(); signal != nullptr) {
                        signal->emit(type);
                    }
                }),
            std::memory_order_release);
    }
    ~Slot() {
        delete _node->_transferHandler.exchange(nullptr,
                                                std::memory_order_release);
    }

private:
    template <bool should_emplace>
    std::atomic<Signal::Handler*>* loadHandler(SignalType type) {
        uint8_t index = std::countr_zero(static_cast<uint64_t>(type));
        if (auto iter = _handlerTables.find(index);
            iter != _handlerTables.end()) {
            return iter->second;
        }
        if constexpr (!should_emplace) {
            return nullptr;
        } else {
            auto handlerManager =
                _node->_handlerManager.load(std::memory_order_acquire);
            auto newManager = new Signal::HandlerManager(index, handlerManager);
            // It's ok here, write only has once thread
            _node->_handlerManager.store(newManager, std::memory_order_release);
            _handlerTables.emplace(index, &newManager->_handler);
            return &newManager->_handler;
        }
    }

    // TODO: may be flatmap better?
    std::unordered_map<uint8_t, std::atomic<Signal::Handler*>*> _handlerTables;
    std::shared_ptr<Signal> _signal;
    Signal::Node* _node;
};

class SignalException : public std::runtime_error {
private:
    SignalType _signal;

public:
    explicit SignalException(SignalType signal, std::string msg = "")
        : std::runtime_error(std::move(msg)), _signal(signal) {}
    SignalType value() const { return _signal; }
};

// those helper cancel functions are used for coroutine await_resume,
// await_suspend & await_ready

struct signal_await {
    signal_await(SignalType sign) : _sign(sign) {}
    template <typename... Args>
    [[nodiscard]] bool suspend(Slot* slot, Args&&... args) noexcept {
        if (slot &&
            !slot->emplace(_sign,
                           std::forward<Args>(args)...)) {  // has canceled
            return false;
        }
        return true;
    }

    void resume(Slot* slot, std::string error_msg = "") {
        if (slot && !slot->clear(_sign) && slot->canceled()) {
            if (error_msg.empty()) {
                error_msg =
                    "async-simple signal triggered and throw exception, signal "
                    "no: " +
                    std::to_string(std::countr_zero(uint64_t{_sign}));
            }
            // TODO: may add option for user to wait signal handler finished?
            throw SignalException{_sign, std::move(error_msg)};
        }
    }

    bool ready(const Slot* slot) noexcept {
        return slot && slot->hasTriggered(_sign);
    }
    SignalType _sign;
};

inline void Signal::Node::invoke(SignalType type, SignalType signal,
                                 Signal* self,
                                 std::atomic<Handler*>& handlerPtr) {
    if (!isMultiTriggerSignal(type)) {
        if (auto* handler = handlerPtr.exchange(&HandlerManager::emittedTag,
                                                std::memory_order_acq_rel);
            handler != nullptr) {
            (*handler)(signal, self);
            delete handler;
        }
    } else {
        if (auto* handler = handlerPtr.load(std::memory_order_acquire);
            handler != nullptr) {
            (*handler)(signal, self);
        }
    }
}

inline void Signal::Node::operator()(SignalType type, Signal* self) {
    // we need call triggered(type) before exchange, which avoid get error
    // filter after filterGuard destructed.
    uint64_t state = type & _filter.load(std::memory_order_acquire);
    auto handlerManager = _handlerManager.load(std::memory_order_acquire);
    if (auto* handler = _transferHandler.load(std::memory_order_acquire);
        handler != nullptr && state) {
        (*handler)(static_cast<SignalType>(state));
    }
    while (state && handlerManager) {
        SignalType nowSignal =
            static_cast<SignalType>(uint64_t{1} << handlerManager->_index);
        if (nowSignal & state) {
            invoke(nowSignal, static_cast<SignalType>(state), self,
                   handlerManager->_handler);
            state ^= nowSignal;
        }
        handlerManager = handlerManager->_next;
    }
}

inline Signal::Node::~Node() {
    if (auto manager = _handlerManager.load(std::memory_order_acquire);
        manager) {
        manager->releaseNodes();
    }
}

inline SignalType Signal::emit(SignalType state) noexcept {
    if (state != SignalType::none) {
        SignalType vaildSignal = UpdateState(_state, state);
        if (vaildSignal) {
            for (Node* node = _slotsHead.load(std::memory_order_acquire);
                 node != nullptr; node = node->_next) {
                (*node)(vaildSignal, this);
            }
        }
        return vaildSignal;
    }
    return SignalType::none;
}

inline void Signal::registSlot(Slot* slot, SignalType filter) {
    auto next_node = _slotsHead.load(std::memory_order_acquire);
    auto* node = new Signal::Node{filter};
    node->_next = next_node;
    while (!_slotsHead.compare_exchange_weak(node->_next, node,
                                             std::memory_order_release,
                                             std::memory_order_relaxed))
        ;
    slot->_node = node;
}

inline Signal::~Signal() {
    if (auto node = _slotsHead.load(std::memory_order_acquire);
        node != nullptr) {
        node->releaseNodes();
    }
}

inline Signal::HandlerManager::~HandlerManager() {
    auto handler = _handler.load(std::memory_order_acquire);
    if (handler != &HandlerManager::emittedTag) {
        delete _handler;
    }
}

};  // namespace async_simple

#endif  // ASYNC_SIMPLE_SIGNAL_H