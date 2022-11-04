## Uthead

async_simple supports C++20 stackless coroutine and stackful coroutine, which is based on context switching.
Uthread is similar to other stackful coroutine, which implements switching by saving the resgister and stack for current stackful coroutine A and resume the register and stack for resuming stackful coroutine B.

The implmentation of Uthread comes from boost library.

### async

It is easy to create uthread just like to create a thread. We could create a stackful coroutine by `async` interface. `async` would receive a lambda function as the root function and an executor to specify the context the stackful coroutine would execute in.

- Create a Uthread to a specifiy executor. The `async` function would return immediately.

```cpp
#include <async_simple/uthread/Async.h>
using namespace async_simple;

uthread::async<uthread::Launch::Schedule>(<lambda>, ex);
```

- Create a Uthread and execute it in current thread. The `async` function would return after the created Uthread checked out.

```cpp
#include <async_simple/uthread/Async.h>
using namespace async_simple;

uthread::async<uthread::Launch::Current>(<lambda>, ex);
```

### CollectAll

async_simple offers `collectAll` interface for Uthread just as stackless coroutine Lazy and Future/Promise to execture Uthread concurrently.

In the following example, `F` is a C++ lambda function, the type of returned value `value `is `std::vector<T>`, `T` is the return type of `F`. If `T` is `void`, `collectAll` would return `async_simple::Unit`.

- Specify multiple Uthread to executre concurrently. This requires that there is no data race in the multiple Uthread.

```cpp
#include <async_simple/uthread/Collect.h>
using namespace async_simple;

std::vector<F> v;
auto res = uthread::collectAll<uthread::Launch::Schedule>(v.begin(), v.end(), ex);
```

- Specify multiple Uthread to execute in current thread. This is used when there are data races between the functions.

```cpp
#include <async_simple/uthread/Collect.h>
using namespace async_simple;

std::vector<F> v;
auto res = uthread::collectAll<uthread::Launch::Current>(v.begin(), v.end(), ex);
```

### latch

The semantic of `latch` is similar with [std::latch](https://en.cppreference.com/w/cpp/thread/latch). In fact, the design of `latch` looks at `std::latch`.
`latch` is used to synchronize multiple Uthread. The Uthread waiting for `latch` would be suspended until the count is zero by calling `downCount()`.

```cpp
#include <async_simple/uthread/Latch.h>
using namespace async_simple;
static constexpr std::size_t kCount = 10;

uthread::async<Launch::Schedule>([ex]() {
    uthread::Latch latch(kCount);
    for (auto i = 0; i < kCount; ++i) {
        uthread::async<uthread::Launch::Schedule>([latchPtr = &latch]() {
            latchPtr->downCount();
        }, ex);
    }
    latchPtr->await(ex);
}, ex);
```

### await

Uthread and Lazy are two different coroutines. And we find that they could be used together in practice. See [HybridCoro](./HybridCoro.md). Here we shows how Uthread calls Lazy by `await` interface.

- Call non-member functions/lambda with return type `Lazy<T>`.

```cpp
#include <async_simple/uthread/Await.h>
using namespace async_simple;

coro::Lazy<T> foo(Ts&&...) { ... }
uthread::await(ex, foo, Ts&&...);

auto lambda = [](Ts&&...) -> coro::Lazy<T> { ... };
uthread::await(ex, lambda, Ts&&...);
```

- Call member function with return type `Lazy<T>`.

```cpp
#include <async_simple/uthread/Await.h>
using namespace async_simple;

class Foo {
public:
    coro::Lazy<T> bar(Ts&&...) { ... }
};

Foo f;
uthread::await(ex, &Foo::bar, &f, Ts&&...);
```

- Await futures `Future<T>`. Although we implemented `Future<T>::wait`, it wouldn't call stackful switch in/out. We need to call `uthread::await` to trigger stackful switching.

```cpp
#include <async_simple/uthread/Await.h>
using namespace async_simple;

Promise<int> p;
// ...
uthread::await(p.getFuture());
```

## Sanitizer

Newer Compiler-rt will enable `Use-After-Return` by default. But it can't take care of Uthread well, so when we use Uthread with newer compiler-rt, we need to disable `Use-After-Return` explicitly by `-fsanitize-address-use-after-return=never`.
