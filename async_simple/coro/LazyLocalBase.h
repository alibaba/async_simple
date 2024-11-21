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
#ifndef ASYNC_SIMPLE_CORO_LAZYLOCALBASE_H
#define ASYNC_SIMPLE_CORO_LAZYLOCALBASE_H


#ifndef ASYNC_SIMPLE_USE_MODULES
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <cstdint>
#endif  // ASYNC_SIMPLE_USE_MODULES
namespace async_simple::coro {

class LazyLocalBase {
private:
    template <typename Derived>
    friend class LazyLocalBaseImpl;
    LazyLocalBase(void* typeinfo) : _typeinfo(typeinfo){};

public:
    LazyLocalBase(std::nullptr_t) : _typeinfo(nullptr){};
    template <typename T>
    T* dyn_cast() noexcept;
    template <>
    LazyLocalBase* dyn_cast<LazyLocalBase>() noexcept {
        return this;
    }
    bool empty() const noexcept { return _typeinfo == nullptr; }
    virtual const std::type_info* type() { return nullptr; }
    virtual ~LazyLocalBase(){};

protected:
    void* _typeinfo;
};

template <typename Derived>
class LazyLocalBaseImpl : public LazyLocalBase {
public:
    LazyLocalBaseImpl() : LazyLocalBase((void*)&typeTag){};

private:
    struct Unit {};
    friend class LazyLocalBase;
    inline const static Unit typeTag{};
    const std::type_info* type() noexcept override { return &typeid(Derived); }
};

template <typename T>
concept isDerivedFromLazyLocal = std::is_base_of<LazyLocalBaseImpl<T>, T>();

template <typename T>
struct SimpleLazyLocal : public LazyLocalBaseImpl<SimpleLazyLocal<T>> {
    template <typename... Args>
    SimpleLazyLocal(Args&&... args) : localValue(std::forward<Args>(args)...) {}
    T localValue;
};

template <typename T>
T* LazyLocalBase::dyn_cast() noexcept {
    if constexpr (isDerivedFromLazyLocal<T>) {
        if (&T::typeTag == _typeinfo) {
            return static_cast<T*>(this);
        } else {
            return nullptr;
        }
    } else {
        if (&SimpleLazyLocal<T>::typeTag == _typeinfo) {
            return &static_cast<SimpleLazyLocal<T>*>(this)->localValue;
        } else {
            return nullptr;
        }
    }
}
}  // namespace async_simple::coro
#endif