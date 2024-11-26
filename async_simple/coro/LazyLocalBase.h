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

// User can derived user-defined class from Lazy Local variable to implement
// user-define lazy local value by implement static function T::classof(const
// LazyLocalBase*).

// For example:
// struct mylocal : public LazyLocalBase {

//     inline static char tag; // support a not-null unique address for type
//     checking
//     // init LazyLocalBase by unique address
//     mylocal(std::string sv) : LazyLocalBase(&tag), name(std::move(sv)) {}
//     // derived class support implement T::classof(LazyLocalBase*), which
//     check if this object is-a derived class of T static bool
//     classof(const LazyLocalBase*) {
//         return base->getTypeTag() == &tag;
//     }

//     std::string name;

// };
class LazyLocalBase {
protected:
    LazyLocalBase(char* typeinfo) : _typeinfo(typeinfo) {
        assert(typeinfo != nullptr);
    };

public:
    const char* getTypeTag() const noexcept { return _typeinfo; }
    LazyLocalBase() : _typeinfo(nullptr) {}
    virtual ~LazyLocalBase(){};

protected:
    char* _typeinfo;
};

template <typename T>
const T* dynamicCast(const LazyLocalBase* base) noexcept {
    assert(base != nullptr);
    if constexpr (std::is_same_v<T, LazyLocalBase>) {
        return base;
    } else {
        if (T::classof(base)) {
            return static_cast<T*>(base);
        } else {
            return nullptr;
        }
    }
}
template <typename T>
T* dynamicCast(LazyLocalBase* base) noexcept {
    assert(base != nullptr);
    if constexpr (std::is_same_v<T, LazyLocalBase>) {
        return base;
    } else {
        if (T::classof(base)) {
            return static_cast<T*>(base);
        } else {
            return nullptr;
        }
    }
}
// namespace async_simple::coro
}  // namespace async_simple::coro
#endif