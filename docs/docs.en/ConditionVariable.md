# ConditionVariable

async_simple provide `ConditionVariable`, looks like `std::condition_variable`. It's used for Lazy stackless coroutine. The other coroutines will not be blocked when current running coroutine invoke `ConditionVariable::wait()`.

## Usage

### ConditionVariable

- `ConditionVariable` need a mutex to work. Any type of mutex that has `coLock()/unlock()` public method can be used. A `SpinLock/ConditionVariable` example as below.

```c++
#include <async_simple/coro/ConditionVariable.h>

SpinLock mtx;
ConditionVariable<SpinLock> cond;
int value = 0;

Lazy<> producer() {
  co_await mtx.coLock();
  value++;
  cond.notify();
  mtx.unlock();
  co_return;
}

Lazy<> consumer() {
  co_await mtx.coLock();
  co_await cond.wait(mtx, [&] { return value > 0; });
  mtx.unlock();
  assert(value > 0);
  co_return;
}
```

- Note: Unlike `std::condition_vaiable`, `ConditionVariable<Lock>` is a template class, and the parameter of `wait` is `Lock` instead of `std::unique_lock<std::mutex>`, we Also provides `notifyAll` and `nofifyOne` interfaces, `notify` and `notifyAll` have the same semantics.

### Notifier
- We can use Notifier to replace ConditionVariable when the condition is simply true or false. The Notifier never depend on any mutex.

```c++
#include <async_simple/coro/ConditionVariable.h>

Notifier notifier;
bool ready = false;

Lazy<> producer() {
  ready = true;
  notifier.notify();
  co_return;
}

Lazy<> consumer() {
  co_await notifier.wait();
  assert(ready);
  co_return;
}
```

The multi coroutines can be blocked on the same ConditionVariable or Notifier. All of the coroutines will be resumed when `ConditionVariable::notify` invoked.
