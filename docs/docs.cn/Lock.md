# 协程锁

## Spin Lock

async_simple提供的自旋锁包含Lazy无栈协程版本以及普通线程版本。普通线程版本和大多数自旋锁类似，当线程暂时无法抢占到锁时会持续自旋直到获取锁为止。Lazy无栈协程版本自旋锁在机制上类似，但是当前协程持续无法获取到锁时，当前协程会被重新调度以避免长时间自旋阻塞当前线程上其他协程。

### 协程版用法

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

### 线程版用法

线程版会使得当前线程死等，尽量避免在协程环境中使用。

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

### 控制自旋

尽管协程版自旋锁长时间获取不到锁会主动让出。但是重新调度会使得获取锁时间变得更加不确定，这取决于Executor调度行为。当临界区范围过大时，频繁让出可能引起性能下降。此时可以增大自旋次数来减少协程主动让出频率。

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

## SharedLock

async_simple提供了协程版本的读写锁读写锁，用于处理读操作较重并且读远远多于写（达到两个数量级及以上）的情况。

SharedLock内部由一个SpinLock和两个ConditionVarible组成，默认自旋128次。用户可以通过构造函数来改变自旋次数。由于该锁仅在SharedLock内部使用，各临界区已确定且极短，因此我们使用SpinLock而不是Mutex来实现读写锁。

SharedLock的实现不存在写操作饥饿问题。


### 示例代码

```cpp
#include <async_simple/coro/SharedLock.h>
using namespace async_simple::coro;

SharedLock lock;

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

很遗憾的是，SharedLock不能提供一个RAII的lock_guard/unique_lock/scopeLock，来自动加锁/解锁。这是因为c++的对象的析构函数不能是无栈协程，由于因此RAII对象无法在析构时解锁。

因此，当代码逻辑复杂/可能抛出异常时，很可能会在某个分支忘记解锁SharedLock，请务必小心。