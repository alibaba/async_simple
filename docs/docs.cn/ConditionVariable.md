# 条件变量

async\_simple中实现的条件变量类似于C++标准库中`std::condition_variable`。主要区别在于async_simple中条件变量面向Lazy协程。协程阻塞在条件变量上时，不会阻塞住当前线程。用于多个协程之间交互协作。基于协程版条件变量，多个协程可以实现典型生产者消费者模型。

## 用法

### ConditionVariable

- 条件变量需要配合一个互斥锁来使用。互斥锁用于保证更改条件和查询条件这些操作原子性。可以用任意一种带有`coLock()/unlock()`方法的互斥锁配合条件变量使用。下面展示了用自旋锁配合条件变量用法。

```c++
#include <async_simple/coro/ConditionVariable.h>

using namespace async_simple::coro;

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

- 注意：与`std::condition_vaiable`不同的是，`ConditionVariable<Lock>`是一个模板类，`wait`的参数是`Lock`而非`std::unique_lock<std::mutex>`，我们也提供了`notifyAll`和`nofifyOne`接口，`notify`和`notifyAll`语义相同。

### Notifier
- 当条件变量中条件只是true/false这种情况时，条件变量可以退化为Notifier。Notifier不依赖外部互斥锁。

```c++
#include <async_simple/coro/ConditionVariable.h>

using namespace async_simple::coro;

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

不论条件变量还是Notifier都允许多个协程同时wait在上面。一旦`notify()`被调用，所有之前阻塞住的协程都将依次被唤醒执行。
