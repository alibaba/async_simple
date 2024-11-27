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
#ifndef ASYNC_SIMPLE_CANCELLATION_H
#define ASYNC_SIMPLE_CANCELLATION_H

#include <thread>
#ifndef ASYNC_SIMPLE_USE_MODULES

#include <assert.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <system_error>
#include <utility>

#include "async_simple/Common.h"
#include "util/move_only_function.h"
#endif  // ASYNC_SIMPLE_USE_MODULES

namespace async_simple {

enum class CancellationType : uint64_t {
    none = 0,
    // low bits reserve for user-define cancellation type
    terminal = uint64_t{1} << 63,  // cancellation type: terminal
    all = UINT64_MAX,
};

class CancellationSlot;

using CancellationHandler = util::move_only_function<void(CancellationType)>;

class CancellationSignal
    : public std::enable_shared_from_this<CancellationSignal> {
    struct PrivateConstructTag {};
    struct Node {
        Node(CancellationType filter)
            : _handler(nullptr), _filter(filter), _next(nullptr) {}
        ~Node();
        void releaseNodes() {
            for (Node *node = this->_next, *next_node; node != nullptr;
                 node = next_node) {
                next_node = node->_next;
                delete node;
            }
            delete this;
        }
        bool canceled(CancellationType state) const noexcept {
            return static_cast<uint64_t>(state) &
                   static_cast<uint64_t>(
                       _filter.load(std::memory_order_acquire));
        }
        void setFilter(CancellationType filter) noexcept {
            _filter.store(filter, std::memory_order_release);
        }
        CancellationType getFilter() const noexcept {
            return _filter.load(std::memory_order_relaxed);
        }
        void operator()(CancellationType type);
        std::atomic<CancellationHandler*> _handler;
        std::atomic<CancellationType> _filter;
        std::atomic<bool> _isHandlerFinished;
        Node* _next;
    };
    static inline Node emittedTag{CancellationType::none};
    bool registSlot(CancellationSlot* slot, CancellationType filter);

public:
    CancellationSignal(PrivateConstructTag){};
    // Signal can only trigger once. This function emit a signal to binding
    // slots, then execute the slot callback functions. Thread-safe, emit twice
    // or more times will return false.
    bool emit(CancellationType state) noexcept;
    // Check if signal has emited.
    bool hasEmited() noexcept {
        return _state.load(std::memory_order::acquire) !=
               CancellationType::none;
    }
    friend class CancellationSlot;
    // Return cancellation type now.
    CancellationType state() const noexcept {
        return _state.load(std::memory_order_acquire);
    }
    // Create CancellationSignal by Factory function.
    static std::shared_ptr<CancellationSignal> create() {
        return std::make_shared<CancellationSignal>(PrivateConstructTag{});
    }
    ~CancellationSignal();

private:
    std::atomic<CancellationType> _state;
    std::atomic<Node*> _slotsHead;
    std::atomic<Node*> _emittedNodes;
};

class CancellationSlot {
public:
    // regist a slot to signal, if signal isn't emitted now, the slot will be
    // connected to signal.
    CancellationSlot(CancellationSignal* signal,
                     CancellationType filter = CancellationType::all)
        : _signal(signal->shared_from_this()), _hasEmplaced(false) {
        if (!signal->registSlot(this, filter)) {
            _signal = nullptr;
        }
    };
    CancellationSlot(const CancellationSlot&) = delete;

public:
    // Register a signal handler. Returns false if the cancellation signal has
    // already been triggered or the slot is disconnected.
    template <typename... Args>
    [[nodiscard]] bool emplace(Args&&... args) {
        if (!connected()) {
            return false;
        }
        auto handler =
            std::make_unique<CancellationHandler>(std::forward<Args>(args)...);
        logicAssert(static_cast<bool>(*handler),
                    "CancellationSlot dont allow emplace empty handler");
        auto oldHandler = _node->_handler.load(std::memory_order_relaxed);
        if (oldHandler == &emittedTag) {
            return false;
        }
        auto new_handler = handler.release();
        if (!_node->_handler.compare_exchange_strong(
                oldHandler, new_handler, std::memory_order_release)) {
            // if slot is canceled and new emplace handler doesn't exec, return
            // false
            assert(oldHandler == &emittedTag);
            delete new_handler;
            return false;
        }
        _hasEmplaced = true;
        delete oldHandler;
        return true;
    }
    // Clear the signal handler. If return false, it indicates
    // that the signal handler has been executed, the slot is disconnected, or
    // handler is empty
    bool clear() {
        if (!connected()) {
            return false;
        }
        auto oldHandler = _node->_handler.load(std::memory_order_relaxed);
        if (oldHandler == &emittedTag || oldHandler == nullptr) {
            return false;
        }
        if (!_node->_handler.compare_exchange_strong(
                oldHandler, nullptr, std::memory_order_release)) {
            assert(oldHandler == &emittedTag);
            return false;
        }
        delete oldHandler;
        return true;
    }

    class FilterGuard {
    public:
        ~FilterGuard() noexcept {
            if (_slot)
                _slot->_node->setFilter(_oldFilter);
        }
        operator bool() const noexcept { return _slot != nullptr; }

    private:
        FilterGuard() : _slot(nullptr), _oldFilter(CancellationType::none){};
        FilterGuard(CancellationSlot* slot, CancellationType newFilter) noexcept
            : _slot(slot), _oldFilter(_slot->getFilter()) {
            auto filter = static_cast<uint64_t>(_oldFilter) &
                          static_cast<uint64_t>(newFilter);
            if (!_slot->connected()) {
                _slot = nullptr;
                return;
            }
            if (auto handler =
                    _slot->_node->_handler.load(std::memory_order_acquire);
                handler == &emittedTag) {
                _slot = nullptr;
                return;
            }
            _slot->_node->setFilter(static_cast<CancellationType>(filter));
            auto new_handler =
                _slot->_node->_handler.load(std::memory_order_acquire);
            if (new_handler == &emittedTag &&
                !_slot->_node->canceled(_slot->signal()->state())) {
                // handler executed before set filter
                _slot->_node->setFilter(_oldFilter);
                _slot = nullptr;
            }
        }
        friend class CancellationSlot;
        CancellationSlot* _slot;
        CancellationType _oldFilter;
    };

    // Filter signals within the specified scope. If signal type & filter is 0,
    // then the signal type will not be triggered within this scope. Nested
    // filters are allowed.
    // FilterGuard could convert to bool. If slot has canceled before
    // addScopedFilter, return false.
    //
    [[nodiscard]] FilterGuard addScopedFilter(
        CancellationType filter) noexcept {
        if (connected()) {
            return FilterGuard{this, filter};
        } else {
            return FilterGuard{};
        }
    }

    // Get the current scope's filter.
    // if slot is not connect, always return CancellationType::all
    CancellationType getFilter() const noexcept {
        if (connected()) {
            return _node->getFilter();
        } else {
            return CancellationType::all;
        }
    }

    // Check whether the filtered cancellation signal has emitted.
    // if return true, the signal has emitted(but handler may hasn't execute
    // now) if slot is not connect to signal, always return true;
    bool canceled() const noexcept {
        if (connected()) {
            return _node->canceled(signal()->state());
        } else {
            return true;
        }
    }

    // Check whether the emplaced handler has started executing.
    // if slot is not connect to signal, or slot never emplaced, it will return
    // false;
    bool hasStartExecute() const noexcept {
        return _hasEmplaced &&
               _node->_handler.load(std::memory_order_acquire) == &emittedTag;
    }

    // Check whether the emplaced handler has finished executing.
    // if slot is not connect to signal, or slot never emplaced, it will return
    // false;
    bool hasFinishExecute() const noexcept {
        return _hasEmplaced &&
               _node->_isHandlerFinished.load(std::memory_order_acquire);
    }

    // The slot holds ownership of the corresponding signal, so the signal's
    // lifetime is always no shorter than the slot's. To extend the signal's
    // lifetime, you can call signal()->shared_from_this(), or start a new
    // coroutine with the signal.
    // return nullptr when signal isn't connect to slot.
    CancellationSignal* signal() const noexcept { return _signal.get(); }

    friend class CancellationSignal;

    // those helper static functions are used for coroutine await_resume,
    // await_suspend & await_ready

    template <typename... Args>
    [[nodiscard]] static bool suspend(CancellationSlot* slot,
                                      Args&&... args) noexcept {
        if (slot &&
            !slot->emplace(std::forward<Args>(args)...)) {  // has canceled
            return false;
        }
        return true;
    }

    template <bool syncAwaitCancelHandlerFinished = false>
    static void resume(CancellationSlot* slot) {
        if (slot && !slot->clear()) {
            if constexpr (syncAwaitCancelHandlerFinished) {
                while (slot->hasFinishExecute()) {
                    std::this_thread::yield();
                }
            }
            throw std::system_error{
                std::make_error_code(std::errc::operation_canceled)};
        }
    }

    static bool ready(CancellationSlot* slot) noexcept {
        return slot && slot->canceled();
    }

private:
    // return true if cancellationSlot connect to a signal
    bool connected() const noexcept {
        // if singal() is nullptr, _node should also be nullptr
        assert(((signal() == nullptr) ^ (_node == nullptr)) == false);
        return signal() != nullptr;
    }

    static inline CancellationHandler emittedTag;
    std::shared_ptr<CancellationSignal> _signal;
    CancellationSignal::Node* _node;
    bool _hasEmplaced;
};

inline void CancellationSignal::Node::operator()(CancellationType type) {
    // we need call canceled(type) before exchange, which avoid get error filter
    // after filterGuard destructed.
    bool is_canceled = canceled(type);
    CancellationHandler* handler = _handler.exchange(
        &CancellationSlot::emittedTag, std::memory_order_acq_rel);
    if (handler != nullptr && is_canceled) {
        (*handler)(type);
    }
    _isHandlerFinished.store(true, std::memory_order_release);
    delete handler;
}

inline CancellationSignal::Node::~Node() {
    if (_handler != &CancellationSlot::emittedTag)
        delete _handler;
}

inline bool CancellationSignal::emit(CancellationType state) noexcept {
    CancellationType expected = CancellationType::none;
    if (state != expected) {
        if (_state.compare_exchange_strong(expected, state,
                                           std::memory_order_release)) {
            _emittedNodes.store(
                _slotsHead.exchange(&emittedTag, std::memory_order_acq_rel));
            for (Node* node = _emittedNodes; node != nullptr;
                 node = node->_next) {
                (*node)(state);
            }
            return true;
        } else {
            return false;
        }
    }
    return true;
}

inline bool CancellationSignal::registSlot(CancellationSlot* slot,
                                           CancellationType filter) {
    auto next_node = _slotsHead.load(std::memory_order_relaxed);
    if (next_node == &emittedTag) {
        slot->_node = nullptr;
        return false;
    }
    auto* node = new CancellationSignal::Node{filter};
    node->_next = next_node;
    while (node->_next != &emittedTag &&
           !_slotsHead.compare_exchange_weak(node->_next, node,
                                             std::memory_order_release,
                                             std::memory_order_relaxed))
        ;
    if (node->_next == &emittedTag) {
        slot->_node = nullptr;
        delete node;
        return false;
    }
    slot->_node = node;
    return true;
}

inline CancellationSignal::~CancellationSignal() {
    if (auto node = _slotsHead.load(std::memory_order_acquire);
        node != nullptr && node != &emittedTag) {
        node->releaseNodes();
    }
    if (auto node = _emittedNodes.load(std::memory_order_acquire); node) {
        assert(node != &emittedTag);
        node->releaseNodes();
    }
}

};  // namespace async_simple

#endif  // ASYNC_SIMPLE_CANCELLATION_H