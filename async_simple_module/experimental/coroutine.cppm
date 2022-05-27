// -*- C++ -*-
//===----------------------------- coroutine -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

/**
    experimental/coroutine synopsis

// C++next

namespace std {
namespace experimental {
inline namespace coroutines_v1 {

  // 18.11.1 coroutine traits
template <typename R, typename... ArgTypes>
class coroutine_traits;
// 18.11.2 coroutine handle
template <typename Promise = void>
class coroutine_handle;
// 18.11.2.7 comparison operators:
bool operator==(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator!=(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator<(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator<=(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator>=(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
bool operator>(coroutine_handle<> x, coroutine_handle<> y) _NOEXCEPT;
// 18.11.3 trivial awaitables
struct suspend_never;
struct suspend_always;
// 18.11.2.8 hash support:
template <class T> struct hash;
template <class P> struct hash<coroutine_handle<P>>;

} // namespace coroutines_v1
} // namespace experimental
} // namespace std

 */

module;

#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>  // for hash<T*>
#include <new>
#include <type_traits>

// This coroutine module is an indepentence implementation, it does not depend on the implementation in stdlib

export module experimental.coroutine;

export namespace async_simple {
namespace detail {

template <class>
struct __void_t {
    typedef void type;
};

}  // namespace detail
}  // namespace async_simple

namespace std {
namespace experimental {

template <class _Tp, class = void>
struct __coroutine_traits_sfinae {};

template <class _Tp>
struct __coroutine_traits_sfinae<_Tp, typename async_simple::detail::__void_t<
                                          typename _Tp::promise_type>::type> {
    using promise_type = typename _Tp::promise_type;
};

export template <typename _Ret, typename... _Args>
struct coroutine_traits : public __coroutine_traits_sfinae<_Ret> {};

export template <typename _Promise = void>
class coroutine_handle;

export template <>
class coroutine_handle<void> {
public:
    constexpr coroutine_handle() noexcept : __handle_(nullptr) {}

    constexpr coroutine_handle(std::nullptr_t) noexcept : __handle_(nullptr) {}

    coroutine_handle& operator=(std::nullptr_t) noexcept {
        __handle_ = nullptr;
        return *this;
    }

    constexpr void* address() const noexcept { return __handle_; }

    constexpr explicit operator bool() const noexcept { return __handle_; }

    void operator()() { resume(); }

    void resume() {
        assert(__is_suspended());
        assert(!done());
        __builtin_coro_resume(__handle_);
    }

    void destroy() {
        assert(__is_suspended());
        __builtin_coro_destroy(__handle_);
    }

    bool done() const {
        assert(__is_suspended());
        return __builtin_coro_done(__handle_);
    }

public:
    static coroutine_handle from_address(void* __addr) noexcept {
        coroutine_handle __tmp;
        __tmp.__handle_ = __addr;
        return __tmp;
    }

    // FIXME: Should from_address(nullptr) be allowed?

    static coroutine_handle from_address(std::nullptr_t) noexcept {
        return coroutine_handle(nullptr);
    }

    template <class _Tp, bool _CallIsValid = false>
    static coroutine_handle from_address(_Tp*) {
        static_assert(
            _CallIsValid,
            "coroutine_handle<void>::from_address cannot be called with "
            "non-void pointers");
    }

private:
    bool __is_suspended() const noexcept {
        // FIXME actually implement a check for if the coro is suspended.
        return __handle_;
    }

    template <class _PromiseT>
    friend class coroutine_handle;
    void* __handle_;
};

// 18.11.2.7 comparison operators:
export inline bool operator==(coroutine_handle<> __x,
                       coroutine_handle<> __y) noexcept {
    return __x.address() == __y.address();
}
export inline bool operator!=(coroutine_handle<> __x,
                       coroutine_handle<> __y) noexcept {
    return !(__x == __y);
}
export inline bool operator<(coroutine_handle<> __x, coroutine_handle<> __y) noexcept {
    return std::less<void*>()(__x.address(), __y.address());
}
export inline bool operator>(coroutine_handle<> __x, coroutine_handle<> __y) noexcept {
    return __y < __x;
}
export inline bool operator<=(coroutine_handle<> __x,
                       coroutine_handle<> __y) noexcept {
    return !(__x > __y);
}
export inline bool operator>=(coroutine_handle<> __x,
                       coroutine_handle<> __y) noexcept {
    return !(__x < __y);
}

export template <typename _Promise>
class coroutine_handle : public coroutine_handle<> {
    using _Base = coroutine_handle<>;

public:
    // 18.11.2.1 construct/reset
    using coroutine_handle<>::coroutine_handle;

    coroutine_handle& operator=(std::nullptr_t) noexcept {
        _Base::operator=(nullptr);
        return *this;
    }

    _Promise& promise() const {
        return *static_cast<_Promise*>(
            __builtin_coro_promise(this->__handle_, alignof(_Promise), false));
    }

public:
    static coroutine_handle from_address(void* __addr) noexcept {
        coroutine_handle __tmp;
        __tmp.__handle_ = __addr;
        return __tmp;
    }

    // NOTE: this overload isn't required by the standard but is needed so
    // the deleted _Promise* overload doesn't make from_address(nullptr)
    // ambiguous.
    // FIXME: should from_address work with nullptr?

    static coroutine_handle from_address(std::nullptr_t) noexcept {
        return coroutine_handle(nullptr);
    }

    template <class _Tp, bool _CallIsValid = false>
    static coroutine_handle from_address(_Tp*) {
        static_assert(_CallIsValid,
                      "coroutine_handle<promise_type>::from_address cannot be "
                      "called with "
                      "non-void pointers");
    }

    template <bool _CallIsValid = false>
    static coroutine_handle from_address(_Promise*) {
        static_assert(
            _CallIsValid,
            "coroutine_handle<promise_type>::from_address cannot be used with "
            "pointers to the coroutine's promise type; use 'from_promise' "
            "instead");
    }

    static coroutine_handle from_promise(_Promise& __promise) noexcept {
        typedef typename std::remove_cv<_Promise>::type _RawPromise;
        coroutine_handle __tmp;
        __tmp.__handle_ = __builtin_coro_promise(
            std::addressof(const_cast<_RawPromise&>(__promise)),
            alignof(_Promise), true);
        return __tmp;
    }
};

#if __has_builtin(__builtin_coro_noop)
export struct noop_coroutine_promise {};

export template <>
class coroutine_handle<noop_coroutine_promise> : public coroutine_handle<> {
    using _Base = coroutine_handle<>;
    using _Promise = noop_coroutine_promise;

public:
    _Promise& promise() const {
        return *static_cast<_Promise*>(
            __builtin_coro_promise(this->__handle_, alignof(_Promise), false));
    }

    constexpr explicit operator bool() const noexcept { return true; }
    constexpr bool done() const noexcept { return false; }

    constexpr void operator()() const noexcept {}
    constexpr void resume() const noexcept {}
    constexpr void destroy() const noexcept {}

private:
    friend coroutine_handle<noop_coroutine_promise> noop_coroutine() noexcept;

    coroutine_handle() noexcept { this->__handle_ = __builtin_coro_noop(); }
};

export using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;

export inline noop_coroutine_handle noop_coroutine() noexcept {
    return noop_coroutine_handle();
}
#endif  // __has_builtin(__builtin_coro_noop)

export struct suspend_never {
    bool await_ready() const noexcept { return true; }
    void await_suspend(coroutine_handle<>) const noexcept {}
    void await_resume() const noexcept {}
};

export struct suspend_always {
    bool await_ready() const noexcept { return false; }
    void await_suspend(coroutine_handle<>) const noexcept {}
    void await_resume() const noexcept {}
};

}  // namespace experimental
}  // namespace std

namespace std {

export template <class _Tp>
struct hash<std::experimental::coroutine_handle<_Tp> > {
    using __arg_type = std::experimental::coroutine_handle<_Tp>;

    size_t operator()(__arg_type const& __v) const noexcept {
        return hash<void*>()(__v.address());
    }
};

}  // namespace std

namespace async_simple {
namespace coro {
export template <typename T = void>
using CoroHandle = std::experimental::coroutine_handle<T>;
}
}  // namespace async_simple

