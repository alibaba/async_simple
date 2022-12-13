# Lazy

Lazy 由 C++20 无栈协程实现。一个 Lazy 代表一个惰性求值的计算任务。

## 使用 Lazy

想要使用 Lazy，需要先 `#inlude <async_simple/coro/Lazy.h>`, 再实现一个返回类型为 `Lazy<T>` 的协程函数即可。例如：

```C++
#include <async_simple/coro/Lazy.h>
Lazy<int> task1(int x) {
    co_return x; // 带有 co_return 的函数是协程函数。
}
```

在 Lazy 中也可以 `co_await` 其他 `awaitable` 对象：

```C++
#include <async_simple/coro/Lazy.h>
Lazy<int> task2(int x) {
    co_await std::suspend_always{};
    co_return x; // 带有 co_return 的函数是协程函数。
}
```

## 启动方式

一个 Lazy 应该以 `co_await`、 `syncAwait` 以及 `.start(callback)` 方式启动。

### co_await 启动

例如:

```C++
#include <async_simple/coro/Lazy.h>
Lazy<int> task1(int x) {
    co_return x; // 带有 co_return 的函数是协程函数。
}
Lazy<> task2() {
    auto t = task1(10);
    auto x = co_await t; // 开始协程task1 't' 的执行
    assert(x == 10);
}
```

`co_await` Lazy 时会触发 [对称变换](https://lewissbaker.github.io/2020/05/11/understanding_symmetric_transfer)
将当前协程挂起并立即执行被 `co_await` 的 Lazy。当被 `co_await` 的 Lazy 执行完毕后，会使用 `对称变换` 将当前挂起的协程唤醒。
并将 `Lazy<T>` 中所包含的值作为 `co_await` 表达式的值返回。

需要注意，我们 `co_await` 一个 Lazy 时并不能假设 `co_await` 后的语句一定会被执行。原因例如：
- 被等待的任务一直没有完成。
- 调度器实现有 bug，被提交到调度器的任务最后不一定会被执行。
- 被等待的任务执行过程中出现了异常，在这种情况下 `co_await` 会直接将该异常返回到当前正在等待的 Lazy，而不会执行之后的语句了。

同时需要注意，使用 `co_await` 启动 Lazy 需要当前函数也为 C++20 无栈协程。

### .start(callback) 启动

例如：
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

`Lazy<T>::start(callback)` 中的 `callback` 需要是一个接受 `Try<T>` 类型参数的 [callable](https://en.cppreference.com/w/cpp/named_req/Callable) 对象。

`Lazy<T>::start(callback)` 方法被调用后会立即开始执行 Lazy，并在 Lazy 执行完成后将 Lazy 的执行结果传入 `callback` 中进行执行。
在设计上，`start` 是非阻塞异步调用接口。语义上，用户可以认为 `start` 在被调用后立即返回。用户不应该假设 `start`  在被调用后何时返回。这是由 Lazy 的执行情况决定的。

对于不需要 `callback` 的情况，用户可以写:
```C++
task().start([](auto&&){});
```

### syncAwait 启动

例如：
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
    auto value = syncAwait(task2()); // 同步等 task2 执行完成
}
```

`syncAwait` 在启动 Lazy 的同时会阻塞当前进程直到 `syncAwait` 的 Lazy 执行完毕。`syncAwait` 属于同步阻塞式调用。

## 取值与异常处理

对于 `Lazy<T>` 类型的对象 `task`，`co_await task` 的返回类型为 `T`。如果 `task` 的执行过程中发生了异常，那么 `co_await task` 会直接抛出该异常。

例如：

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
    std::cout << "Result: " << res << "\n"; // res 的值会是 -1。
}
```

需要注意，虽然 `co_await task` 时有可能会抛出异常，但总是使用 `try...catch` 包裹 `co_await` 并不是推荐的做法。一方面这样写确实很麻烦，另一方面，当 `co_await`抛出异常时，协程的框架保证了会由 `co_await` 所在的协程处理抛出的异常。

例如：

```C++
Lazy<int> foo() {
    throw std::runtime_error("test");
    co_return 1;
}
Lazy<int> bar() {
    int res;
    res = co_await foo();
    assert(false); // 不会执行，因为上一行会抛出异常。

    co_return res;
}
Lazy<int> baz() {
    co_return co_await bar();
}
void normal() {
    try {
        syncAwait(baz());
    } catch(...) {
        // 我们可以在这里捕获到异常。
    }
}
void normal2() {
    baz().start([](Try<int> result){
        if (result.hasError())
            std::cout << "baz has error!!\n"; 
    });
}
```

当由 `co_await` 组成的任务链的某一层出现异常时，异常会一层层地往上抛。对于 `syncAwait` 形式的启动方式，我们可以在 `syncAwait` 处加上 `try..catch` 语句。而对于实际中更常用的 `.start` 方式，我们可以在回调处通过 `Try` 对象获取是否出现异常。

如果不想让异常直接往上抛，而希望直接处理，则可以使用 `coAwaitTry` 接口。例如：

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

对于 `Lazy<T>` 类型的对象 `task`, `co_await task.coAwaitTry()` 的返回值类型为 `Try<T>`。

# RescheduleLazy

RescheduleLazy 在语义上是绑定了 Executor 的 Lazy。RescheduleLazy 在 `co_await` Lazy 时并不会使用对称变换立即执行 Lazy，而是将唤醒 Lazy 的任务提交到 RescheduleLazy 绑定的 Executor 中。因为语义上 RescheduleLazy 也是 Lazy，所以 RescheduleLazy 也能使用 Lazy 的接口 `co_await`、`.start` 和 `syncAwait`.

## 创建 RescheduleLazy

RescheduleLazy 不能直接创建，也不能像 Lazy 一样作为协程的返回类型。RescheduleLazy 只能通过 Lazy 的 via 接口创建，例如:

```C++
void foo() {
    executors::SimpleExecutor e1(1);
    auto addOne = [&](int x) -> Lazy<int> {
        auto tmp = co_await getValue(x);
        co_return tmp + 2;
    };
    RescheduleLazy Scheduled = addOne().via(&e1);
    syncAwait(Scheduled); // 由调度器决定何时开始 `addOne` 的执行
}
```

## 调度器的传递

在编写计算任务时，我们都只使用 Lazy。当我们想要为一个计算任务指定调度器时，我们只需要在任务开始阶段指定调度器即可。
指定的调度器会随着 `co_await` 一路传递下去。
例如：

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

在上面的例子中，`task1...task4` 代表一个计算任务的调用链，其结果都是以 Lazy 表示的。
在 func 中，在创建了 `task4` 的 Lazy 后，为其指定了调度器 `e` 得到了一个 RescheduleLazy。
之后调用 `.start` 时，不仅 `task4` 具体何时执行是由调度器决定的。
包括 `task3`, `task2`, `task1` 何时执行，也都是由调度器决定的。

总之，当我们需要为多个 Lazy 组成的任务链指定调度器时，我们只需要在任务链的开头指定调度器就好了。

# Yield

有时我们可能希望主动让出一个 Lazy 协程的执行，将执行资源交给其他任务。（例如我们发现当前协程执行的时间太长了）
我们可以通过在 Lazy 中 `co_await async_simple::coro::Yield();` 来做到这一点。

# 获取当前调度器

有时我们可能需要获取当前协程的调度器，我们可以通过在 Lazy 中 `co_await async_simple::CurrentExecutor{};` 来获取调度器。

# Collect

## collectAll

当我们有多个计算任务时，一个很常见的需求是等待所有计算任务完成以获取所有任务的结果后再进行进一步的计算。
我们可以使用 `collectAll` 接口完成这个需求。例如：

```C++
Lazy<int> foo() {
    std::vector<Lazy<int>> input;
    input.push_back(ComputingTask(1));
    input.push_back(ComputingTask(2));

    vector<Try<int>> out = co_await collectAll(std::move(input));

    co_return out[0].value() + out[1].value();
}
```

`collectAll` 表示一个开始执行参数中所有 Lazy 的任务。`collectAll` 是一个协程，需要使用 `co_await` 以获取结果。


### 参数要求

collectAll 接受两种类型的参数：
- 参数类型：`std::vector<Lazy<T>>`。 返回类型： `std::vector<Try<T>>`。
- 参数类型: `Lazy<T1>, Lazy<T2>, Lazy<T3>, ...`。 返回类型： `std::tuple<Try<T1>, Try<T2>, Try<T3>, ...>`。

第二种参数类型的例子为：

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

## 其他接口
### collectAllPara

如果 colletAll 的所有参数都是 Lazy  而非 RescheduleLazy，那么 collectAll 会单线程地执行每个 Lazy。
有两个办法可以解决这个问题：
- 为所以参数绑定 Lazy 使其成为 RescheduleLazy。
- 使用 collectAllPara。

使用 collectAllPara 需要注意其所在协程必须要绑定调度器，否则依然是单线程执行所有 Lazy。

例如：
```C++
Lazy<int> foo() {
    std::vector<Lazy<int>> input;
    input.push_back(ComputingTask(1));
    input.push_back(ComputingTask(2));

    vector<Try<int>> out = co_await collectAllPara(std::move(input));

    co_return out[0].value() + out[1].value();
}
void bar() {
    // auto t = syncAwait(foo()); // 错误！foo() 没有绑定调度器！依然会串行执行。
    executors::SimpleExecutor e1(1);
    auto t = syncAwait(foo().via(&e1)); // 正确，提前为 foo() 绑定调度器
}
```

collectAllPara 的参数与返回类型和 collectAll 相同。

### collectAllWindowed

当我们需要分批次的执行并发任务时，我们可以使用 collectAllWindowed。
collectAllWindowed 的参数列表及语义为为:
- `size_t maxConcurrency`。每个批次所规定的任务数。
- `bool yield`。在执行完当前批次后，是否需要 yield 使当前协程退出，将控制权交给其他协程。
- `std::vector<Lazy<T>> lazys`。所有需要执行的任务。

例如：

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

当我们只需要等待所有计算任务中的任意一个任务完成时，我们可以使用 `collectAny` 来获取第一个完成的任务结果。此时其他尚未完成的任务的结果会被忽略。

#### 参数类型与行为

collectAny 接受两种类型的参数：
- 参数类型：`std::vector<LazyType<T>>`。 返回类型： `Lazy<CollectAnyResult<T>>`。
- 参数类型: `LazyType<T1>, LazyType<T2>, LazyType<T3>, ...`。 返回类型： `std::variant<Try<T1>, Try<T2>, Try<T3>, ...>`。

其中 LazyType 应为 `Lazy<T>` 或 `RescheduleLazy<T>`。

当 LazyType 为 `Lazy<T>` 时，`collectAny` 的行为为在当前线程直接执行该协程任务，当该协程任务挂起后，再迭代到下一个任务。当 LazyType 为 `RescheduleLazy<T>` 时，`collectAny` 的行为为在该 `RescheduleLazy<T>` 指定的调度器中提交对应的任务后，再迭代到下一个任务。

选择使用 `Lazy<T>` 还是 `RescheduleLazy<T>` 需要根据场景以及调度器实现的不同来进行选择。例如当任务到达第一个可能的暂停点相对较短时，使用 `Lazy<T>` 可以节约调度的开销，同时可能可以触发短路以节约计算。例如：

```C++
bool should_get_value();
int default_value();
Lazy<int> conditionalWait() {
    if (should_get_value())
        co_return co_await get_remote_value();
    co_return default_value();
}
Lazy<int> getAnyConditionalValue() {
    std::vector<Lazy<int>> input;
    for (unsigned i = 0; i < 1000; i++)
        input.push_back(conditionalWait());

    auto any_result = co_await collectAny(std::move(input));
    assert(!any_result.hasError());
    co_return any_result.value();
}
```

例如对于这个例子来说，每个任务到达第一个暂停点的路径都很短，同时有可能触发短路。例如可能在执行到第 1 个任务时 `should_get_value()` 就返回 `false`，此时可以不用将协程挂起而获得第一个任务的值，于是之后的 999 个任务都可以不执行。更具体的例子是每个任务对应一个带有 Buffer 的 IO 任务，每个任务会先在 Buffer 中查询值，当在 Buffer 中不存在对应的值时，就会异步查询一个耗时较长的 IO 任务，此时 Lazy 任务就会挂起，`collectAny` 就会开始执行下一个任务。

但当任务到达第一个暂停点的路径相对较长或者说每个任务相对较重时，用 `RescheduleLazy<T>` 可能比较好。例如：

```C++
void prepare_for_long_time();
Lazy<int> another_long_computing();
Lazy<int> long_computing() {
    prepare_for_long_time();
    co_return co_await another_long_computing();
}
Lazy<int> getAnyConditionalValue(Executor* e) {
    std::vector<RescheduleLazy<int>> input;
    for (unsigned i = 0; i < 1000; i++)
        input.push_back(conditionalWait().via(e));

    auto any_result = co_await collectAny(std::move(input));
    assert(!any_result.hasError());
    co_return any_result.value();
}
```

在这个 case 中，每一个任务可能都比较重。此时用 `Lazy<T>` 的话就有可能导致某个任务执行时间过长而迟迟不让出资源导致其他有可能有机会更早获得结果的任务无法开启。在这个情况下，用 `RescheduleLazy<T>` 可能就会更好一些。

#### CollectAnyResult

CollectAnyResult 的数据结构为：

```C++
template <typename T>
struct CollectAnyResult {
    size_t _idx;
    Try<T> _value;

    size_t index() const;
    bool hasError() const;
    // Require hasError() == true. Otherwise it is UB to call
    // this method.
    std::exception_ptr getException() const;
    // Require hasError() == false. Otherwise it is UB to call
    // value() method.
    const T& value() const&;
    T& value() &;
    T&& value() &&;
    const T&& value() const&&;
};
```

其中 `_idx` 表示第一个完成的任务的编号，可使用 `index()` 成员方法获取。`_value` 表示第一个完成的任务的值，
可使用 `hasError()` 方法判断该任务是否成功；在该任务失败的情况下，可以使用 `getException()` 方法获取其异常；在该任务成功的情况下，可使用 `value()` 成员方法获取其值。

例子：

```C++
Lazy<void> foo() {
    std::vector<Lazy<int>> input;
    input.push_back(ComputingTask(1));
    input.push_back(ComputingTask(2));

    auto any_result = co_await collectAny(std::move(input));
    std::cout << "The index of the first task completed is " << any_result.index() << "\n";
    if (any_result.hasError())
        std::cout << "It failed.\n";
    else
        std::cout << "Its result: " << any_result.value() << "\n";
}
Lazy<void> foo_var() {
  auto res = co_await collectAny(ComputingTask<int>(1),ComputingTask<int>(2),ComputingTask<double>(3.14f));
  std::cout<< "Index: " << res.index();
  std::visit([](auto &&value){
    std::cout<<"Value: "<< value <<std::endl;
  }, res);
}
```
