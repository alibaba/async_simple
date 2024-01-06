## Future

Future/Promise are classic asynchronous components. It is implemented in async_simple too. Although Future/Promise is relatively raw, it may not be so user friendly if we're writing a lot of asynchronous code. Future/Promise are good communicating channel for stackful coroutines, stackless coroutines and normal functions.

## Simple Use

Declare a Promise:

```cpp
async_simple::Promise<int> p;
```

We could use `async_simple::Unit` if we don't want a value type.

```cpp
async_simple::Promise<async_simple::Unit> p;
```

We can get future by `Promise::getFuture()`.

```cpp
async_simple::Future<int> f = p.getFuture();
```

We can wait for Promise to set the value by `Future::wait()`

```cpp
f.wait();
```

After `wait()`, we can use `Future::value()` to get the value.

```cpp
f.wait();
int v = f.value();
```

We can use `Future::get()` to wait and get the value.

```cpp
int v = f.get();
```

When there may be an exception, we need to use `wait()` and `result()` to get the wrapped result. `Try<T>`

```cpp
f.wait();
Try<int> t = f.result();
if (t.hasError())
    {}// HandleException
else
    {}// get value
```

Promise can set the value by `Promise::setValue()` and `Promise::setException()`.

```cpp
p.setValue(43);
p.setValue(t); // t is Try<int>
p.setException(ex_ptr); // ex_ptr is std::exception_ptr
```

Similar with other Future/Promise, the Future/Promsie in async_simple are one one matched and we can set the value at most once.

## Chaining Call

We could enable chaining call for Future by `Future::thenValue(Callable&&)` or `Future::thenTry(Callable&&)`.For `Future::thenValue(Callable&&)` Callable should accept one argument  `T`. For `Future::thenTry(Callable)`, Callable should accept one argument `Try<T>`.

`Future::thenValue(Callable&&)` and `Future::thenTry(Callable&&)` will pass the value to Callable after Future gets the value.

When Callable returns `Future<X>`,`Future::thenValue(Callable&&)` and `Future::thenTry(Callable&&)` returns `Future<X>`.When Callable returns `X` and `X` is not Future,`Future::thenValue(Callable&&)` and `Future::thenTry(Callable&&)` returns `Future<X>`:

```cpp
Promise<int> p;
auto future = p.getFuture();
auto f = std::move(future)
                 .thenTry([&output0, record](Try<int>&& t) {
                     record(0);
                     output0 = t.value();
                     return t.value() + 100;
                 })
                 .thenTry([&output1, &executor, record](Try<int>&& t) {
                     record(1);
                     output1 = t.value();
                     Promise<int> p;
                     auto f = p.getFuture().via(&executor);
                     p.setValue(t.value() + 10);
                     return f;
                 })
                 .thenValue([&output2, record](int x) {
                     record(2);
                     output2 = x;
                     return std::to_string(x);
                 })
                 .thenValue([](string&& s) { return 1111.0; });
p.setValue(1000);
f.wait();
```

If we want the executor to decide when will the Callable be called, we can use `Future::via()` to specify the Executor.

```cpp
Promise<int> p;
auto future = p.getFuture().via(&executor); // the type of executor is derived type of async_simple::Executor
auto f = std::move(future)
                 .thenTry([&output0, record](Try<int>&& t) {
                     record(0);
                     output0 = t.value();
                     return t.value() + 100;
                 })
                 .thenTry([&output1, &executor, record](Try<int>&& t) {
                     record(1);
                     output1 = t.value();
                     Promise<int> p;
                     auto f = p.getFuture().via(&executor);
                     p.setValue(t.value() + 10);
                     return f;
                 })
                 .thenValue([&output2, record](int x) {
                     record(2);
                     output2 = x;
                     return std::to_string(x);
                 })
                 .thenValue([](string&& s) { return 1111.0; });
p.setValue(1000);
f.wait();
```
We can also enable chaining call for Future by `Future::then(Callable&&)`. It will decide to call `Future::thenTry` or `Future::thenValue` based on the parameters of `Callable`.

```cpp
Promise<int> p;
auto f = p.getFuture()
                .then([](int i) { return i * 2; })
                .then([](Try<int> t) { return t.value() * 2; })
                .then([](int i) { return i; });
p.setValue(1);
f.wait();
```

## Get value from Lazy

Need to include `"async_simple/coro/FutureAwaiter.h"`.
In the Lazy,we can `co_await` a Future directly.

```cpp
Promise<int> pr;
auto fut = pr.getFuture();
sum(1, 1, [pr = std::move(pr)](int val) mutable { pr.setValue(val); });
std::this_thread::sleep_for(std::chrono::milliseconds(500));
auto val = co_await std::move(fut);
```

If the Future has set Executor already, the Executor would decide when will the Lazy to be resumed. Note that it wouldn't change the executor the Lazy lives in.

When binding Executor, `co_await` Future will resume Lazy through the `Executor.checkin` interface by default, and users can explicitly specify that Future resume Lazy through the `Executor.schedule` interface by `co_await ResumeBySchedule(std::move(future))`.

If the Lazy has set Executor already and we want to set that executor for the Future, we can make it by `co_await CurrentExecutor{};`.

```cpp
Promise<int> pr;
auto fut = pr.getFuture().via(co_await CurrentExecutor{};);
sum(1, 1, [pr = std::move(pr)](int val) mutable { pr.setValue(val); });
std::this_thread::sleep_for(std::chrono::milliseconds(500));
auto val = co_await std::move(fut);
```

## Get value from Uthread

Need to include `"async_simple/uthread/Await.h"`.

In the uthread, we can get the value  of Future from `async_simple::uthread::await(Future<T>&&)`

```cpp
auto v = async_simple::uthread::await(std::move(fut));
```
