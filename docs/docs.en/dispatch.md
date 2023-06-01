# dispatch

By using the dispatch function, the current Lazy can be scheduled to execute on the specified Executor, which provides a convenient interface for users. For example, when a user needs to schedule an operation to a different Executor for execution, if there is no dispatch function, a new Lazy needs to be generated and necessary resources transferred to the new Lazy. The dispatch function can reduce these losses.

## Usage
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

## Illustrate
* If the executor specified in the dispatch is the current executor, execution will continue without causing a rescheduling.
* If the dispatch specified Executor fails to schedule (the `schedule` interface returns false), an exception `std::runtime_error("dispatch to executor failed")`will be thrown.
