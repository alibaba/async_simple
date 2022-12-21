# Semaphore
Analogous to [`std::counting_semaphore`](https://en.cppreference.com/w/cpp/thread/counting_semaphore), but it's for `lazy`.

## Usage

### Notifier
```c++
#include <async_simple/coro/Semaphore>

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
```c++
#include <async_simple/coro/Semaphore>

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
