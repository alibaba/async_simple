## Future

Future/Promise 是经典的异步组件。在 async_simple 中也有其实现。虽然因为 Future/Promise 有较为原始、可能陷入回调地狱等问题，使用 Future/Promise 写大量的异步程序可能没那么方便。但使用 Future/Promise 作为有栈/无栈协程，协程/普通函数之间沟通的桥梁还是非常合适的。

## 简单使用

经典的 Future/Promise 的使用案例为：先声明一个 Promise：

```C++
async_simple::Promise<int> p;
```

当不需要类型参数时，可以使用 `async_simple::Unit` 类型，作为占位符：

```C++
async_simple::Promise<async_simple::Unit> p;
```

通过 `Promise::getFuture()` 方法获取 Future：

```C++
async_simple::Future<int> f = p.getFuture();
```

之后可以通过 `Future::wait()` 等待 Promise 设置值：

```C++
f.wait();
```

当 `wait()` 结束后，可以使用 `Future::value()` 获取值。

```C++
f.wait();
int v = f.value();
```

也可以使用 `Future::get()` 方法，等待并获取值：

```C++
int v = f.get();
```

当 Future 的结果可能为异常时，需要 `wait()` 后使用 `result()` 获取 `Try<T>`：

```C++
f.wait();
Try<int> t = f.result();
if (t.hasError())
    {}// HandleException
else
    {}// get value
```

Promise 可以通过 `Promise::setValue()` 和 `Promise::setException()` 为 Future 设置值：

```C++
p.setValue(43);
p.setValue(t); // t is Try<int>
p.setException(ex_ptr); // ex_ptr is std::exception_ptr
```

与其他 Future/Promise 模型一样，async_simple 的 Future 与 Promise 一一对应，同时只能获取值一次。

## 链式调用

Future 可以通过 `Future::thenValue(Callable&&)` 或 `Future::thenTry(Callable&&)` 来进行链式调用。`Future::thenValue(Callable&&)` 的参数是一个接收 `T` 的可调用对象。`Future::thenTry(Callable)` 的参数是一个接收 `Try<T>` 的可调用对象。

`Future::thenValue(Callable&&)` 和 `Future::thenTry(Callable&&)` 的语义是当 Future 获取到值之后将值传到 `Callable` 中进行调用。是异步等待的语义。

`Future::thenValue(Callable&&)` 和 `Future::thenTry(Callable&&)` 的返回类型也是 Future，所以可以进行链式调用。当 Callable 的返回类型为 `Future<X>` 时，`Future::thenValue(Callable&&)` 和 `Future::thenTry(Callable&&)` 的返回类型是 `Future<X>`。当 Callable 的返回类型是 `X` 且 `X` 不是 Future 时，`Future::thenValue(Callable&&)` 和 `Future::thenTry(Callable&&)` 的返回类型是 `Future<X>`：

```C++
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

如果我们希望由调度器决定 Callable 的执行时机，我们可以使用 `Future::via()` 来指定调度器：

```C++
Promise<int> p;
auto future = p.getFuture().via(&executor); // executor 的类型是 async_simple::Executor 的子类
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

## 与无栈协程交互

需要 include `"async_simple/coro/FutureAwaiter.h"`。
在无栈协程中，我们可以直接 `co_await` 一个 Future 来获取其值：

```C++
Promise<int> pr;
auto fut = pr.getFuture();
sum(1, 1, [pr = std::move(pr)](int val) mutable { pr.setValue(val); });
std::this_thread::sleep_for(std::chrono::milliseconds(500));
auto val = co_await std::move(fut);
```

如果 Future 已经设置过 Executor，那么此时当前协程的 Resumption 由该调度器控制。

如果当前协程已设置了调度器，Future 未设置调度器且我们希望由该调度器介入，我们可以通过 `co_await CurrentExecutor{};` 来做到：

```C++
Promise<int> pr;
auto fut = pr.getFuture().via(co_await CurrentExecutor{};);
sum(1, 1, [pr = std::move(pr)](int val) mutable { pr.setValue(val); });
std::this_thread::sleep_for(std::chrono::milliseconds(500));
auto val = co_await std::move(fut);
```

## 与有栈协程交互

需要 include `"async_simple/uthread/Await.h"`。
在有栈协程中，我们可以通过 `async_simple::uthread::await(Future<T>&&)` 来获取一个有栈协程的值。

```C++
auto v = async_simple::uthread::await(std::move(fut));
```
