# Lazy

Lazy is implemented by C++20 stackless coroutine. A Lazy is a lazy-evaluated computational task.

## Use Lazy

We need to include `<async_simple/coro/Lazy.h>` first to use Lazy. And we need to implement a function whose return type is `Lazy<T>`. Like:

```C++
#include <async_simple/coro/Lazy.h>
Lazy<int> task1(int x) {
    co_return x; // A function with co_return is a coroutine function.
}
```

We could `co_await` other `awaitable` objects in Lazy:

```C++
#include <async_simple/coro/Lazy.h>
Lazy<int> task2(int x) {
    co_await std::suspend_always{};
    co_return x;
}
```

## Start Lazy

We could start a Lazy by `co_await`, `syncAwait` and `.start(callback)`.

### co_await

For example:

```C++
#include <async_simple/coro/Lazy.h>
Lazy<int> task1(int x) {
    co_return x;
}
Lazy<> task2() {
    auto t = task1(10);
    auto x = co_await t; // start coroutine 't'.
    assert(x == 10);
}
```

When we `co_await` a Lazy, [symmetric transfer](https://lewissbaker.github.io/2020/05/11/understanding_symmetric_transfer) would happen.
Symmetric transfer would suspend the current coroutine and execute the `co_await`ed coroutine immediately. If the `co_await`ed coroutine is a Lazy, it would resume the current coroutine by symmetric transfer when the `co_await`ed coroutine is done. 
The value returned by `co_await` expression is wrapped in `Lazy<T>`.

Note that, we couldn't assume that the statements after a `co_await expression` would run finally. It is possible if:
- The waited task doesn't complete all the way.
- There is an bug in the scheduler. The task submitted to the scheduler wouldn't be promised to schedule.
- There is an exception happened in the waited task. In this case, the current coroutine would return to its caller instead of executing the following statments.

Another thing we need to note is that the function contains a `co_await` would be a C++20 stackless coroutine too.

### .start(callback)

For example:
```C++
#include <async_simple/coro/Lazy.h>
#include <iostream>
Lazy<int> task1(int x) {
    co_return x;
}
Lazy<> task2() {
    auto t = task1(10);
    auto x = co_await t;
    assert(x == 10);
}
void func() {
    task2().start([](Try<void> Result){
        if (Result.hasError())
            std::cout << "Error Happened in task2.\n";
        else
            std::cout << "task2 completed successfully.\n";
    });
}
```

The `callback` in `Lazy<T>::start(callback)` need to be a [callable](https://en.cppreference.com/w/cpp/named_req/Callable) object which accepts an `Try<T>` argument.

`Lazy<T>::start(callback)` would execute the corresponding Lazy immediately. After the Lazy is completed, its result would be forwarded to the `callback`.
By design, `start` should be a non-blocking asynchronous interface. Semantically, user could image `start` would return immediately. User shouldn't assume when `start` would return. It depends on how the Lazy would execute actually.

In case the `callback` isn't needed, we could write:
```C++
task().start([](auto&&){});
```

### syncAwait

For example:

```C++
#include <async_simple/coro/Lazy.h>
Lazy<int> task1(int x) {
    co_return x;
}
Lazy<> task2() {
    auto t = task1(10);
    auto x = co_await t;
    assert(x == 10);
}
void func() {
    auto value = syncAwait(task2()); // Wait for task2() synchronously.
}
```

`syncAwait` would block the current process until the waited Lazy complete. `syncAwait` is a synchronous blocking interface.

### Get the value and exception handling

For the object `task` whose type is `Lazy<T>`, the type of `co_await task` would be `T`. If there is an exception in `task`, `co_await task` would throw the exception.

For example:

```C++
Lazy<int> foo() {
    throw std::runtime_error("test");
    co_return 1;
}
Lazy<int> bar() {
    int res;
    try {
        res = co_await foo();
    } catch(...) {
        std::cout << "error happened in foo. Set result to -1.\n";
        res = -1;
    }

    co_return res;
}
void baz() {
    auto res = syncAwait(bar());
    std::cout << "Result: " << res << "\n"; // res is -1.
}
```

Note that it is not a good practice to wrap `co_await` by `try...catch` statement all the time. One the one hand, it is inconvenient. On the other hand, the current coroutine would handle the unhandled_exception by the design of coroutine. For the example of `Lazy`, in case of an unhandled exception happens, the exception would be stored into the current Lazy.

For example:

```C++
Lazy<int> foo() {
    throw std::runtime_error("test");
    co_return 1;
}
Lazy<int> bar() {
    int res;
    res = co_await foo();
    assert(false); // Wouldn't execute

    co_return res;
}
Lazy<int> baz() {
    co_return co_await bar();
}
void normal() {
    try {
        syncAwait(baz());
    } catch(...) {
        // We could catch the exception here.
    }
}
void normal2() {
    baz().start([](Try<int> result){
        if (result.hasError())
            std::cout << "baz has error!!\n"; 
    });
}
```

If there is an exception happened in the chain of Lazies, the exception would be forwarded to the root caller. If the Lazy is invoked by `syncAwait`, we could use `try..catch` to catch the exception. For the use of `.start`, we could detect the exception by the `Try` argument in the callback.

If we want to handle the exception in place when we awaits exception, we could use `coAwaitTry` method. For example:

```C++
Lazy<int> foo() {
    throw std::runtime_error("test");
    co_return 1;
}
Lazy<int> bar() {
    Try<int> res = co_await foo().coAwaitTry();
    if (res.hasError()) {
        std::exception_ptr error = res.getException();
        // calculating error.
    }
    co_return res.value();
}
```

For an object `task` with type `Lazy<T>`, the type of expression `co_await task.coAwaitTry()` would be `Try<T>`.

# RescheduleLazy

Semantically, RescheduleLazy is a Lazy with an executor. Unlike Lazy, `co_await` a RescheduleLazy wouldn't invoke symmetric transformation. It would submit the task to resume the RescheduleLazy to the corresponding executor. Since RescheduleLazy is a Lazy semantically. So RescheduleLazy would be able to use the methods as Lazy does: `co_await`, `.start` and `.syncAwait`.

## Get RescheduleLazy

We couldn't create RescheduleLazy directly. And RescheduleLazy couldn't be the return type of a coroutine. We could only get the RescheduleLazy by the `via` method of `Lazy`. For example:

```C++
void foo() {
    executors::SimpleExecutor e1(1);
    auto addOne = [&](int x) -> Lazy<int> {
        auto tmp = co_await getValue(x);
        co_return tmp + 2;
    };
    RescheduleLazy Scheduled = addOne().via(&e1);
    syncAwait(Scheduled); // e1 would decide when would `addOne` execute.
}
```

## Passing Executor

We could use Lazy only to write a seris of computation tasks. And we could assign an executor for the tasks at the start of the root caller. And the executor would be passed along the way the tasks get started.

For example:

```C++
#include <async_simple/coro/Lazy.h>
#include <iostream>
Lazy<int> task1(int x) {
    co_return co_await calculating(x);
}
Lazy<int> task2(int x) {
    co_return co_await task1(x);
}
Lazy<int> task3(int x) {
    co_return co_await task2(x);
}
Lazy<int> task4(int x) {
    co_return co_await task3(x);
}
void func(int x, Executor *e) {
    task4(x).via(e).start([](auto&& result){
        std::cout << "Completed task to calculate x.\n"
                     "Result is " << result << "\n";
    });
}
```

In the above example, `task1...task4` represents a task chain consists of Lazy. We assign an executor `e` for the root of the task chain in `func` then we get a RescheduleLazy. After `start` is called, all of tasks (including `task1..task4`) would scheduled by the executor.

So we could assign the executor at the root the task chain simply.

# Collect

## CollectAll

It is a common need to wait for a lot of tasks. We could use `collectAll` to do this. For example:

```C++
Lazy<int> foo() {
    std::vector<Lazy<int>> input;
    input.push_back(ComputingTask(1));
    input.push_back(ComputingTask(2));

    vector<Try<int>> out = co_await collectAll(std::move(input));

    co_return out[0].value() + out[1].value();
}
```

`collectAll` is a coroutine. `collectAll` represents a task that wait for all the tasks in the input. Since `collectAll` is a coroutine, too. We need to use `co_await` to get the result.

### Arguments

`collectAll` accepts two kinds of argument.
- Argument type: `std::vector<Lazy<T>>`. Return type:  `std::vector<Try<T>>`.
- Argument type: `Lazy<T1>, Lazy<T2>, Lazy<T3>, ...`. Return type: `std::tuple<Try<T1>, Try<T2>, Try<T3>, ...>`.

The example for the second type:

```C++
Lazy<int> computeInt();
Lazy<double> computeDouble();
Lazy<std::string> computeString();
Lazy<> foo() {
    std::tuple<Try<int>, Try<double>, Try<std::string>> Res = 
        co_await collectAll(computeInt(), computeDouble(), computeString());

    Try<int> IntRes = std::get<0>(Res);
    if (IntRes.hasError())
        std::cout << "Error happened in computeInt()\n";
    else
        std::cout << "Result for computeInt: " << IntRes.value() << "\n";
    // ...
}
```

## Other interfaces

### collectAllPara

If all the arguments of `collectAll` is Lazy instead of `RescheduleLazy`, `collectAll` would execute every Lazy serially.
There are two solutions:
- Make every input as `RescheduleLazy`.
- Use `collectAllPara`.

Here let's talk more about `collectAllPara`. Note that the current coroutine needs to have an executor in case we use `collectAllPara`, otherwise all the `Lazy` tasks would be also executed serially.

For example:

```C++
Lazy<int> foo() {
    std::vector<Lazy<int>> input;
    input.push_back(ComputingTask(1));
    input.push_back(ComputingTask(2));

    vector<Try<int>> out = co_await collectAllPara(std::move(input));

    co_return out[0].value() + out[1].value();
}
void bar() {
    // auto t = syncAwait(foo()); // Wrong！foo didn't get executor. The tasks would be executed serially.
    executors::SimpleExecutor e1(1);
    auto t = syncAwait(foo().via(&e1)); // Correct, assign executor in advance
}
```

The argument type and return type of `collectAllPara` is the same with `collectAll`.

### collectAllWindowed

When we need to execute concurrent tasks in batches. We could use `collectAllWindowed`. The arguments type and meaning of `collectAllWindowed` are:
- `size_t maxConcurrency`. The number of tasks in every batch.
- `bool yield`。If the coroutine would suspend when one batch of tasks get completed.
- `std::vector<Lazy<T>> lazys`. All the tasks that need to execute.

For example:

```C++
Lazy<int> sum(std::vector<Try<int>> input);
Lazy<int> batch_sum(size_t total_number, size_t batch_size)  {
    std::vector<Lazy<int>> input;
    for (auto i = 0; i < total_number; i++)
        input.push_back(computingTask());

    auto out = co_await collectAllWindowed(batch_size, true, std::move(input));

    co_return co_await sum(std::move(out));
}
```

### collectAny

Sometimes we need only a result of a lot of tasks. We could use `collectAny` in this case. `collectAny` would return the result of the first task get completed. All other tasks would detached and their results would be ignored.

The argument of `collectAny` is `std::vector<Lazy<T>>`. The return type is `Lazy<CollectAnyResult<T>>`.

The structure of `CollectAnyResult` would be:
```C++
template <typename T>
struct CollectAnyResult<void> {
    size_t _idx;
    Try<T> _value;
};
```

`_idx` means the index of the first completed task. `_value` represents the corresponding value.

For exmaple:

```C++
Lazy<void> foo() {
    std::vector<Lazy<int>> input;
    input.push_back(ComputingTask(1));
    input.push_back(ComputingTask(2));

    auto any_result = co_await collectAny(std::move(input));
    std::cout << "The index of the first task completed is " << any_result._idx << "\n";
    if (any_result._value.hasError())
        std::cout << "It failed.\n";
    else
        std::cout << "Its result: " << any_result._value.value() << "\n";
}
```
