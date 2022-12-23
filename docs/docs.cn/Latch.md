# Latch

async_simple中实现的`Latch`类似于C++标准库中`std::latch`。主要区别在于async_simple中条件变量面向Lazy协程。  
`Latch`是`std::size_t`类型的向下计数器，它能用于同步`Lazy`。在创建时初始化计数器的值。当计数器减为0时，协程将会被挂起并且切换到其他协程去运行。

## 用法
```c++
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

```c++
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

## 避免内存泄漏

当在协程中使用 `co_await Latch::wait()` 时，程序员需要保证该 Latch 的 counter 在程序执行期间总会减到 0。否则程序会出现内存泄漏现象。