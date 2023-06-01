# Executor

Executor是调度协程的关键组件。绝大多数开源协程框架提供内置的调度器，一般包含线程池和调度策略。基于这些协程框架开发时，用户不得不抛弃原有调度器。考虑到这一点，async_simple 设计一套抽象API接口，解耦合了Executor与协程。用户实现这些抽象API接口即可将自己的调度器接入 async_simple，Lazy和Uthread协程在运行时则通过这些接口被调度执行。

## 使用Executor

让协程运行在指定的调度器中非常简单，只需要创建协程时传递Executor给协程即可。在Lazy中通过`via()/setEx()`可以传递Executor；在Uthread中设置`async()`的Executor参数传递。

```cpp
Executor e;

// lazy
co_await f().via(&e).start([](){});
co_await f().setEx(&e);

// uthread
uthread::async<uthread::Launch::Schedule>(<lambda>, &e);
```

具体可以查看Lazy和Uthread相关文档。

## 实现Executor

### 接口定义

Executor定义了如下相关接口。

```cpp
class Executor {
public:
    using Func = std::function<void()>;
    using Context = void *;
    
    virtual bool schedule(Func func) = 0;
    virtual bool currentThreadInExecutor() const = 0;
    virtual ExecutorStat stat() const = 0;
    virtual Context checkout();
    virtual bool checkin(Func func, Context ctx, ScheduleOptions opts);
    virtual IOExecutor* getIOExecutor() = 0;
```

- `virtual bool schedule(Func func) = 0;` 接口接收一个lambda函数进行调度执行。实现时将该lambda调度到任意一个线程执行即可。当 `schedule(Func)` 返回 `true` 时，该调度器实现需要保证 func 一定会被执行。否则被提交到调度器的 Lazy 任务可能会导致程序出现内存泄漏问题。
- `virtual bool currentThreadInExecutor() const = 0;` 接口用于检查调用者是否运行在当前Executor中。实现时判断当前线程是否隶属于Executor。
- `virtual ExecutorStat stat() const = 0;` 接口获取当前Executor状态信息。实现时可以继承ExecutorStat，添加更多状态信息方便用户调试和监控Executor。
- `virtual Context checkout();` 接口获取当前执行上下文信息。实现时一般可以直接返回当前线程在Executor中唯一标识id。
- `virtual bool checkin(Func func, Context ctx, ScheduleOptions opts);` 接口用于调度lambda函数到特定id的线程上执行。ctx来自`checkout()`。
  - `checkout()/checkin()`主要用于协程挂起前记录当前线程，协程恢复时，调度回挂起前所运行的线程。避免可能的数据竞争问题。
  - 当`opts.prompt`为 false 时，checkin 中不可原地执行 func。
- `virtual IOExecutor* getIOExecutor() = 0;` 接口返回IOExecutor，用于异步提交IO请求。
  - 继承实现IOExecutor中submitIO相关接口，用于对接不同异步IO引擎。IOExecutor接口简单，这里不展开阐述。

### SimpleExecutor

SimpleExecutor 为 async_simple 单元测试中使用的 Executor。SimpleExecutor 实现前面提到的Executor接口，包含`ThreadPool.h`中固定线程数的线程池，还包含 SimpleIOExecutor 用于异步提交本地IO请求。用户可以参考 SimpleExecutor 实现来接入业务的私有 Executor。

- 注意：`SimpleExecutor/SimpleIOExecutor/ThreadPool.h`设计为单元测试使用，不建议用于生产环境使用。从代码目录executors可找到完整代码。
- alibaba内部私有Executor目前暂未开源。
