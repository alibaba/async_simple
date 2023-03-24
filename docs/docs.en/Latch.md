# Latch 

async_simple provide `Latch`, looks like `std::latch`. It's used for Lazy stackless coroutine.  
The latch class is a downward counter of type std::size_t which can be
used to synchronize coroutines. The value of the counter is initialized on creation. Coroutines may block on the latch until the counter is decremented to zero. It will suspend the current coroutine and switch to other coroutines
to run.

## Usage
```cpp
#include "async_simple/coro/Latch.h"

using namespace async_simple::coro;

Latch work_done(3);

Lazy<> work1() {
    //...
    co_await work_done.count_down();
}

Lazy<> work2() {
    //...
    co_await work_done.count_down();
}

Lazy<> work3() {
    //...
    co_await work_done.count_down();
}

Lazy<> wait_lazy() {
    // ...
    co_await work_done.wait();
}

```

```cpp
#include "async_simple/coro/Latch.h"

using namespace async_simple::coro;

Latch work_begin(3);

Lazy<> work() {
    // Synchronize
    co_await arrive_and_wait();
    // ...
}

Lazy<> ready() {
    for (int i = 0; i < 3; ++i) {
        work().via(&ex).detach();
    }
    // ...
}
```

## Avoid memory leaking 

When we `co_await Latch::wait()` in a coroutine, we need to be sure that the counter of the latch will be reduced to zero before the end of the execution of the program. Otherwise, the memomry will leak.

