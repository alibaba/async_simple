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
#include <cassert>
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
    LazyLocalBase(char* typeinfo) : _typeinfo(typeinfo) {
        assert(typeinfo != nullptr);
    };

public:
    static bool classof(LazyLocalBase*) noexcept { return true; }
    const char* getTypeTag() const noexcept { return _typeinfo; }
    LazyLocalBase() : _typeinfo(nullptr) {}
    virtual ~LazyLocalBase(){};

protected:
    char* _typeinfo;
};

template <typename T>
concept isDerivedFromLazyLocal = std::is_base_of_v<LazyLocalBase, T>;

template <isDerivedFromLazyLocal T>
T* dynamicCast(LazyLocalBase* base) noexcept {
    if (base == nullptr) {
        return nullptr;
    } else if (T::classof(base)) {
        return static_cast<T*>(base);
    } else {
        return nullptr;
    }
}
}  // namespace async_simple::coro
#endif