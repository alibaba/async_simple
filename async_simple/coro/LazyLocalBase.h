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
#include <cstdint>
#include <type_traits>
#include <utility>
#endif  // ASYNC_SIMPLE_USE_MODULES
namespace async_simple::coro {
class LazyLocalBase;

class LazyLocalBase {
protected:
    template <typename Derived>
    friend class LazyLocalBaseImpl;
    LazyLocalBase(char* typeinfo) : _typeinfo(typeinfo){};

public:
    const char* getTypeTag() const noexcept { return _typeinfo; }
    LazyLocalBase(std::nullptr_t)
        : LazyLocalBase(static_cast<char*>(nullptr)) {}
    virtual ~LazyLocalBase(){};

protected:
    char* _typeinfo;
};

template <typename T>
concept isDerivedFromLazyLocal = std::is_base_of_v<LazyLocalBase, T>;

template <typename T>
struct SimpleLazyLocal : public LazyLocalBase {
    template <typename... Args>
    SimpleLazyLocal(Args&&... args)
        : LazyLocalBase(&typeTag), localValue(std::forward<Args>(args)...) {}
    T localValue;
    static bool classof(LazyLocalBase* base) noexcept {
        return &typeTag == base->getTypeTag();
    }

private:
    inline static char typeTag;
};

template <typename T>
T* dynamicCast(LazyLocalBase* base) noexcept {
    if (base == nullptr) {
        return nullptr;
    }
    if constexpr (std::is_same_v<LazyLocalBase, T>) {
        return base;
    } else if constexpr (isDerivedFromLazyLocal<T>) {
        if (T::classof(base)) {
            return static_cast<T*>(base);
        } else {
            return nullptr;
        }
    } else {
        if (SimpleLazyLocal<T>::classof(base)) {
            return &static_cast<SimpleLazyLocal<T>*>(base)->localValue;
        } else {
            return nullptr;
        }
    }
}
}  // namespace async_simple::coro
#endif