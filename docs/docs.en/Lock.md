# Coroutine Lock

## Spin Lock

async_simple provided `SpinLock` contains coroutine-mode and thread-mode. Use thread-mode might block the other coroutines. We should consider to use coroutine-mode `SpinLock`  in coroutine context firstly, it will not block others.

###  Coroutine-Mode

```cpp
#include <async_simple/coro/SpinLock.h>

SpinLock lock;

Lazy<> doSomething() {
  co_await lock.coLock();
  // critical section
  lock.unlock();
  co_return;
}

Lazy<> doSomethingV2() {
  // ...
  {
    auto scope = co_await lock.coScopedLock();
    // critical section
  }
  co_return;
}
```

### Thread-Mode

```cpp
#include <async_simple/coro/SpinLock.h>

SpinLock lock;

void doSomething() {
  lock.lock();
  // critical section
  lock.unlock();
  co_return;
}

void doSomethingV2() {
  ScopedSpinLock scope(lock);
  // critical section
  co_return;
}
```

### Spin Count

To avoid coroutines yield (re-scheduled by Executor) with high frequency when acquiring lock, We can adjust `SpinLock` spin count to control it.

```cpp
#include <async_simple/coro/SpinLock.h>

SpinLock lock(2048); // Spin Count

Lazy<> doSomething() {
  co_await lock.coLock();
  // critical section
  lock.unlock();
}
```


## Mutex

Coming soon.