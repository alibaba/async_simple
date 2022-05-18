# 协程锁

## Spin Lock

async_simple提供的自旋锁包含Lazy无栈协程版本以及普通线程版本。普通线程版本和大多数自旋锁类似，当线程暂时无法抢占到锁时会持续自旋直到获取锁为止。Lazy无栈协程版本自旋锁在机制上类似，但是当前协程持续无法获取到锁时，当前协程会被重新调度以避免长时间自旋阻塞当前线程上其他协程。

### 协程版用法

```c++
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

### 线程版用法

线程版会使得当前线程死等，尽量避免在协程环境中使用。

```c++
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

### 控制自旋

尽管协程版自旋锁长时间获取不到锁会主动让出。但是重新调度会使得获取锁时间变得更加不确定，这取决于Executor调度行为。当临界区范围过大时，频繁让出可能引起性能下降。此时可以增大自旋次数来减少协程主动让出频率。

```c++
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