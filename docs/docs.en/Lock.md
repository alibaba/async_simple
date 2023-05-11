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

## Condition Varible

Comming soon.

## SharedMutex

async_simple provide a SharedMutex in coroutine for the situation than read operation is much more than write.

There is a `SpinLock` and two `ConditionVarible` in`SharedMutex` implementation. The default spin count is 128, and could be changed by constructor. Because the critical section is only in the internal implementation and it's short, the `SpinLock` is a bbetter choice than mutex.

When one or more reader locks is held a writer gets priority and no more reader locks can be taken while the writer is queued.


### 示例代码

```cpp
#include <async_simple/coro/SharedMutex.h>
using namespace async_simple::coro;

SharedMutex lock;

Lazy<void> read() {
  co_await lock.coLockShared();
  // critical read section
  co_await lock.unlockShared();
  co_return;
}

Lazy<void> write() {
  co_await lock.coLock();
  // critical write section
  co_await lock.unlock();
  co_return;
}
```

### RAII

Unfortunately，`SharedMutex` can't work with a RAII lock_guard/unique_lock/scopeLock helper class, because the destructor of c++ class/struct is not a coroutine function, so it's impossible to `co_await lock.unlock()` in destructor.

When your code has a complex logic/may throw expection, please take care of it and remember to unlock the `SharedMutex`.
