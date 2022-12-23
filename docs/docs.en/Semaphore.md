# Semaphore
Analogous to [`std::counting_semaphore`](https://en.cppreference.com/w/cpp/thread/counting_semaphore), but it's for `lazy`.

## Usage

### Notifier
```c++
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
```c++
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

## Avoid memory leaking 

When we `co_await Semaphore::acquire()` in a coroutine, we need to be sure that the semaphore will be released before the end of the execution of the program. Otherwise, the memomry will leak.
