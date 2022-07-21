module;
#include <coroutine>
export module std:coroutine;

/**
    coroutine synopsis

namespace std {
// [coroutine.traits]
template <class R, class... ArgTypes>
  struct coroutine_traits;
// [coroutine.handle]
template <class Promise = void>
  struct coroutine_handle;
// [coroutine.handle.compare]
constexpr bool operator==(coroutine_handle<> x, coroutine_handle<> y) noexcept;
constexpr strong_ordering operator<=>(coroutine_handle<> x, coroutine_handle<> y) noexcept;
// [coroutine.handle.hash]
template <class T> struct hash;
template <class P> struct hash<coroutine_handle<P>>;
// [coroutine.noop]
struct noop_coroutine_promise;
template<> struct coroutine_handle<noop_coroutine_promise>;
using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;
noop_coroutine_handle noop_coroutine() noexcept;
// [coroutine.trivial.awaitables]
struct suspend_never;
struct suspend_always;
} // namespace std

 */

export namespace std {
    using std::coroutine_traits;
    using std::coroutine_handle;
    using std::noop_coroutine;
    using std::suspend_never;
    using std::suspend_always;
}
