## Executor

Executor is the key component for shceduling coroutine. A lot of the open-sourced coroutine frameworks would offer an executor, which contains thread pool and corresponding scheduling strategy. When user uses these frameworks, user couldn't use his/her original executor. So async_simple designs a set of APIs to decouple coroutine with Executor. Programmers could use their original executor in async_simple by implementing these APIs.

### Use Executor

It is easy to assign a coroutine instance to an executor. The user need to pass the executor to coroutine only. The users could assign an executor to a Lazy by `via/setEx`. They could use `async` to pass Executor to Uthread.

```cpp
Executor e;

// lazy
co_await f().via(&e).start([](){});
co_await f().setEx(&e);

// uthread
uthread::async<uthread::Launch::Schedule>(<lambda>, &e);
```

The readers could read Lazy and Uthread documentation for details.

### Implement Executor
#### Define interface

Executor defines following interfaces.

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

- `Virtual bool schedule(Func func) = 0;`. It would schedule a lambda function to execute. When `scheudle(Func)` returns true, the implementation is required to schedule the lambda to any threads to execute. Otherwise, the unfinished Lazy tasks may result memory leaks.
- `virtual bool currentThreadInExecutor() const = 0;`. It would check if the current thread are in the executor.
- `virtual ExecutorStat stat() const = 0;`. It would return the state information of the executor.
- `virtual Context checkout();` It would be called in case the user (Lazy, Uthread and Future/Promise) want to leave the current thread. The return value of `checkout` is the identity of the current thread. User who want to schedule back to the original thread should use the identity returned from `checkout`.
- `virtual bool checkin(Func func, Context ctx, ScheduleOptions opts);` It would schedule the task (`Func func`) to the specified thread. `ctx` comes from `checkout()`.
    - `checkout()/checkin()` would record the current thread. And it would shcedule the task to the original thread before the resuming the coroutine to avoid potential data race problems.
    - When `opts.prompt` is False, `checkin()` isn't allowed to execute func in place.
- `virtual IOExecutor* getIOExecutor() = 0;`. It would return an IOExecutor, which is used to submit asynchronous IO request.
    - User could implement a class derived from IOExecutor to use different IO Executor.

#### SimpleExecutor

SimpleExecutor is a simple executor used in unit test of async_simple. SimpleExecutor implements the Executor mentioned above including a ThreadPool in `ThreadPool.h` and a simple SimpleIOExecutor which is used for submitting asynchronous IO request. User could implement their executor by mimicing the style of SimpleExecutor.

- Note: `SimpleExecutor/SimpleIOExecutor/ThreadPool.h` is designed to use in unit test. It may not be good enough to use in industry production.
- The executor used in alibaba actually isn't open-sourced yet.
