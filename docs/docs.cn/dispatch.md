# dispatch

通过使用dispatch函数可以将当前Lazy调度到指定Executor上执行，为用户提供了便捷的接口。例如当用户需要将某个操作调度到不同的Executor上执行时，如果没有dispatch函数，则需要生成新的Lazy并将必须的资源转移到新Lazy中以调度执行，dispatch可以减少这些损耗。

## 用法
```c++
#include "async_simple/coro/Dispatch.h"

using namespace async_simple::coro;

Lazy<> work() {
  //...
  Executor* my_ex = co_await CurrentExecutor{};
  assert(my_ex == &executor1);
  co_await dispatch(&executor2);
  my_ex = co_await CurrentExecutor{};
  assert(my_ex == &executor2);
}

syncAwait(work2().via(&executor1));

```

## 说明
* 如果dispatch指定的Executor就是当前Executor，则会继续执行，不会导致重新调度。
* 如果dispatch指定的Executor调度失败（schedule返回false），则会抛出`std::runtime_error("dispatch to executor failed")`异常。
