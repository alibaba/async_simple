## Uthread

async_simple作为C++协程库，不仅支持基于C++20标准的无栈协程，还支持基于上下文交换的有栈协程uthread。
uthread类似于业界其他有栈协程，通过保存当前A协程上下文寄存器和栈，恢复B协程寄存器和栈等信息实现多任务协作式运行。

uthread 来自于 boost 库。

### async

uthread创建非常简单，类似C++线程创建。通过async接口来创建一个协程（下文中均指uthread有栈协程），传入需要跑在该协程中的lambda函数。同时用户需要传入调度器用于调度该协程何时会被执行与唤醒。

- 创建uthread时调度到指定线程池中执行，此时async函数将会立即返回。

```cpp
#include <async_simple/uthread/Async.h>
using namespace async_simple;

uthread::async<uthread::Launch::Schedule>(<lambda>, ex);
```

- 创建uthread时立即在当前线程执行，此时当uthread执行过程中切换出去时async才会返回。

```cpp
#include <async_simple/uthread/Async.h>
using namespace async_simple;

uthread::async<uthread::Launch::Current>(<lambda>, ex);
```

### async

- 我们也提供了类似于`std::async`的接口

```cpp
template <class F, class... Args,
          typename R = std::invoke_result_t<F&&, Args&&...>>
inline Future<R> async(Launch policy, Attribute attr, F&& f, Args&&... args) 
```

#### 参数
```
 policy - 位掩码值，每个单独位控制允许的执行方法
   attr - uthread的属性
      f - 要调用的[可调用](Callable)对象 
args... - 传递给f的参数
```
#### [Callable](https://zh.cppreference.com/w/cpp/named_req/Callable)

#### uthread::Launch
```cpp
enum class Launch {
    Schedule,
    Current,
};
```
- 指定 uthread::async 所指定的任务的的运行策略

| 常量                        | 解释  |
|--------------------------- |--------------|
| uthread::Launch::Schedule  | 指定 std::async 所指定的任务的的运行策略 |
| uthread::Launch::Current   | 创建uthread时立即在当前线程执行，此时当uthread执行过程中切换出去时async才会返回 |

#### uthread::Attribute
```cpp
struct Attribute {
    Executor* ex;
    size_t stack_size = 0;
};
```
- Uthread的属性, ex是[Executor](Executor.md)，stack_size是uthread在运行时使用的堆栈空间.

#### 返回值
- 返回最终将保有该函数调用结果的 Future 。 

#### 例子
```cpp
TEST_F(UthreadTest, testAsync_v2) {
    std::atomic<int> running = 1;
    async(
        Launch::Schedule, Attribute{.ex = &_executor, .stack_size = 4096},
        [](int a, int b) { return a + b; }, 1, 2)
        .thenValue([&running](int ans) {
            EXPECT_EQ(ans, 3);
            running--;
        });
    EXPECT_EQ(await(async(
                  Launch::Current, Attribute{&_executor},
                  [](int a, int b) { return a * b; }, 2, 512)),
              1024);
    while (running) {
    }
}
```

###  collectAll

类似于在async_simple中无栈协程中多个Lazy并发执行，uthread同样提供了collectAll接口实现多个有栈协程并发执行。

下面例子中F为C++ lambda函数，返回值res类型为`std::vector<T>`, T为F函数返回值类型。当T等于void类型时，collectAll 返回
`void` 类型。

- 指定多个协程在多个线程中并行化执行，这种用法一般要求用户多个lambda函数之间不存在数据竞争。

```cpp
#include <async_simple/uthread/Collect.h>
using namespace async_simple;

std::vector<F> v;
auto res = uthread::collectAll<uthread::Launch::Schedule>(v.begin(), v.end(), ex);
```

- 指定多个协程始终在当前线程异步协作式执行，这种一般用在F函数之间存在数据竞争。

```cpp
#include <async_simple/uthread/Collect.h>
using namespace async_simple;

std::vector<F> v;
auto res = uthread::collectAll<uthread::Launch::Current>(v.begin(), v.end(), ex);
```

### latch

latch语义上等价于C++20标准库中[latch](https://en.cppreference.com/w/cpp/thread/latch)，事实上就是参考C++20 latch而设计。uthread中latch用于多个uthread之间同步。等待在latch上的协程将会被挂起，直到多个协程通过`downCount()`将计数减到零时被唤醒。

```cpp
#include <async_simple/uthread/Latch.h>
using namespace async_simple;
static constexpr std::size_t kCount = 10;

uthread::async<Launch::Schedule>([ex]() {
    uthread::Latch latch(kCount);
    for (auto i = 0; i < kCount; ++i) {
        uthread::async<uthread::Launch::Schedule>([latchPtr = &latch]() {
            latchPtr->downCount();
        }, ex);
    }
    latchPtr->await(ex);
}, ex);
```

### await

uthread和lazy作为async_simple中两种不同类型的协程各有优缺，在实际应用中我们发现二者能互补，在混合协程章节中可以看到详细阐述，这里展示uthread如何通过await接口直接调用无栈协程Lazy。

- 调用返回值为`Lazy<T>`的普通函数，lambda函数。

```cpp
#include <async_simple/uthread/Await.h>
using namespace async_simple;

coro::Lazy<T> foo(Ts&&...) { ... }
uthread::await(ex, foo, Ts&&...);

auto lambda = [](Ts&&...) -> coro::Lazy<T> { ... };
uthread::await(ex, lambda, Ts&&...);
```

- 调用返回值为`Lazy<T>`的类成员函数。

```cpp
#include <async_simple/uthread/Await.h>
using namespace async_simple;

class Foo {
public:
    coro::Lazy<T> bar(Ts&&...) { ... }
};

Foo f;
uthread::await(ex, &Foo::bar, &f, Ts&&...);
```

- 异步等待 futures `Future<T>`. 虽然我们实现了 `Future<T>::wait`, 但这个接口不会触发有栈协程切换. 在有栈协程中，我们需要调用 `uthread::await(Future<T>)` 来触发有栈切换。

```cpp
#include <async_simple/uthread/Await.h>
using namespace async_simple;

Promise<int> p;
// ...
uthread::await(p.getFuture());
```

## Sanitizer

高版本的 Compiler-rt 默认会检测 `Use-After-Return` 的情况。而因为 Uthread 不能很好的处理这个情况，所以当我们在开启高版本 Compiler-rt 后使用 Uthread 时，需要使用 `-fsanitize-address-use-after-return=never` 这个选项来禁止检测 `Use-After-Return`.
