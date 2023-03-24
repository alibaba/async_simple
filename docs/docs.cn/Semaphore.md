# 信号量 
类似于[`std::counting_semaphore`](https://zh.cppreference.com/w/cpp/thread/counting_semaphore)，但它用于`Lazy`。

## 用法

### Notifier
```cpp
#include <async_simple/coro/Semaphore>

using namespace async_simple::coro;

BinarySemaphore notifier(0);
bool ready = false;

Lazy<> producer() {
  ready = true;
  co_await notifier.release();
  co_return;
}

Lazy<> consumer() {
  co_await notifier.acquire();
  assert(ready);
  co_return;
}
```

### Mutex
```cpp
#include <async_simple/coro/Semaphore>

using namespace async_simple::coro;

BinarySemaphore sem(1);
int count = 0;

Lazy<> producer() {
  co_await sem.acquire();
  ++count;
  co_await sem.release();
  co_return;
}

Lazy<> consumer() {
  co_await sem.acquire();
  --count;
  co_await sem.release();
  co_return;
}
```

## 避免内存泄漏

当一个协程 `co_await Semaphore::acquire(...);` 时，程序员需要保证该信号量在程序执行期间会被 `release()`，否则因为该协程不能释放导致内存泄漏问题。

