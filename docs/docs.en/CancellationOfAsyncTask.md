We often need to cancel certain related asynchronous tasks once an asynchronous task is completed. For instance, we might want to terminate an IO request promptly if it times out. `async-simple` is based on the signal-slot model and provides a generic, thread-safe, and efficient asynchronous task cancellation mechanism. It also supports structured task concurrency and cancellation.

## CancellationSignal and CancellationSlot

A cancellation signal (`CancellationSignal`) can be registered with multiple cancellation slots (`CancellationSlot`). Typically, when we initiate a group of asynchronous tasks, we can create a signal and multiple slots, passing a slot to each asynchronous task. Each task can obtain the current cancellation status through the slot.

```cpp
// Create a signal using a factory method
std::shared_ptr<CancellationSignal> signal = CancellationSignal::create();
std::vector<std::future<void>> works;
for (int i = 0; i < 10; ++i) {
  // Create a slot for each asynchronous task
  auto slot = std::make_unique<CancellationSlot>(signal.get());
  // Execute task asynchronously
  std::async(std::launch::async, [slot = std::move(slot)] {
      // Manually check cancellation status
      while (!slot->canceled()) {
        // ...
      }
      return;
  });
}
// ...
// Emit cancellation signal
signal->emit(CancellationType::terminal);
for (auto &e : works)
  e.get();
```

Besides directly checking cancellation status, asynchronous tasks can also register a callback function in the cancellation slot before performing asynchronous operations. The slots function should not block and should be able to notify the asynchronous task to stop waiting. The thread that triggers the signal will execute all slots functions. For example, the code below cancels sleep by registering a slot function.

```cpp
// Create a signal using a factory method
std::shared_ptr<CancellationSignal> signal = CancellationSignal::create();
std::vector<std::future<void>> works;
for (int i = 0; i < 10; ++i) {
  // Create a slot for each asynchronous task
  auto slot = std::make_unique<CancellationSlot>(signal.get());
  // Execute task asynchronously
  std::async(std::launch::async, [slot = std::move(slot)] {
      std::unique_ptr<std::promise<void>> p;
      auto f = p->get_future();
      // registering a slot function
      bool ok = slot->emplace([p = std::move(p)](CancellationType type){
          p->set_value();
      });
      if (ok) { // If the cancellation signal has not been triggered
          f.wait_for(1s * (rand() + 1)); // Sleep for a period unless the cancellation signal is triggered.
      }
      slot->clear(); // Clear the callback function
      if (slot->signal()) { // If the slot is bound to the signal
          slot->signal()->emit(CancellationType::terminal); // Trigger the cancellation signal.
      }
      return;
  });
}
// ...
// After the first asynchronous timed task ends, other tasks will end together.
for (auto &e : works)
  e.get();
```

In the above code, we can obtain a pointer to the signal by calling the `signal()` function of the slot. We ensure that the returned pointer is always valid because the lifecycle of the signal is automatically extended until the last bound slot is destroyed.

The above code might call the `emit` function multiple times to attempt to trigger the cancellation signal. We ensure that in such scenarios, the cancellation signal is triggered only once, and subsequent calls to `emit` will return `false`. Additionally, all operations on `CancellationSignal` are thread-safe.

```cpp
class CancellationSignal
    : public std::enable_shared_from_this<CancellationSignal> {
public:
    // Submit a signal, the second call will fail and return false, thread-safe.
    bool emit(CancellationType state) noexcept;
    // Check if the signal has already been submitted, thread-safe.
    bool hasEmitted() noexcept;
    // Get the current cancellation signal, thread-safe
    CancellationType state() const noexcept;
    // The only way to create a signal, returns a shared_ptr
    static std::shared_ptr<CancellationSignal> create();
};
```

Each asynchronous task should hold its own `CancellationSlot`, so slot objects are not thread-safe. Users are prohibited from concurrently calling the public interface provided by the slots.

```cpp
enum class CancellationType;
class CancellationSlot {
    // Bind the signal and slot together. The signal will not be triggered if the cancellation signal type & filter is 0.
    CancellationSlot(CancellationSignal* signal,
                     CancellationType filter = CancellationType::all);
    // Register a signal handling function, returns false if the cancellation signal has already been triggered. Users can register multiple signal handling functions.
    template <typename... Args>
    [[nodiscard]] bool emplace(Args&&... args);
    // Clear the signal handling function, returns false if it has already been executed or never registered.
    bool clear();
    // Filter signals within the specified scope. If cancellation signal type & filter is 0, the signal type will not be triggered within this scope. Nested filters are allowed.
    [[nodiscard]] FilterGuard addScopedFilter(CancellationType filter);
    // Get the current scope's filter
    CancellationType getFilter();
    // Check if the filtered cancellation signal is in a triggered state.
    bool canceled() const noexcept;
    // Check if the callback function has started or finished execution. Unlike `canceled()`, it returns false if the cancellation signal was triggered before registering the callback.
    bool hasStartExecute() const noexcept;
    // This function returns the signal corresponding to the slot. If the cancellation signal was triggered before the slot was constructed, it returns nullptr. Otherwise, it always returns a valid signal pointer. This is because the slot holds the signal's ownership.
    // To extend the signal's lifecycle, you can call signal()->shared_from_this(), or use the signal to start a new coroutine.
    CancellationSignal* signal() const noexcept;
};
enum class CancellationType : uint64_t {
    none = 0,
                                      // Remaining bits reserved for user-defined extensions.
    terminal = 0x8000'0000'0000'0000, // Default cancellation signal
    all = 0xffff'ffff'ffff'ffff,      // Default filter level (no signal filtering)
};
```

## Stackless Coroutine Support

The aforementioned slots and signals are relatively low-level, generic APIs adaptable to various asynchronous scenarios. The `Lazy` stackless coroutine library in `async-simple` provides a series of advanced encapsulations, allowing users to perform structured concurrent task cancellations without worrying about the details.

### collect Function and Structured Concurrency

In general user code, we recommend using `collectAny` and `collectAll` to achieve structured concurrent task cancellations.

#### collectAny

`collectAny` can concurrently execute multiple coroutines and wait for the first one to return. `collectAny` will automatically bind these coroutines to a cancellation signal and send a cancellation signal to the coroutines that have not yet completed once the first one finishes.

For instance, the code below demonstrates a common timeout logic using `collectAny`.

```cpp
Lazy<void> sleep_1s();
Lazy<std::string> async_read();
auto res = co_await collectAny(async_read(), sleep_1s());
if (std::get<res>() == 1) { // timed out!
  // ...
}
else {
  // ...
}
```

`collectAny` supports sending different cancellation signals. The default signal sent is `CancellationType::terminal`. If you don't want `collectAny` to cancel other tasks, you can choose to send `CancellationType::none`.

```cpp
Lazy<void> work1();
Lazy<void> work2();
auto res = co_await collectAny<CancellationType::none>(async_read(), sleep_1s());
if (std::get<res>() == 0) { 
  // work1 finished, work2 will still be working
}
else { 
  // work2 finished, work1 will still be working
}
```

#### collectAll

Similar to `collectAny`, `collectAll` also supports sending cancellation signals when the first task completes. The differences are:

1. By default, `collectAll` sends the `CancellationType::none` signal (so it won't cancel other tasks).
2. `collectAll` waits for all coroutines to complete and retrieves their return values. This simplifies the lifecycle of asynchronous tasks. Moreover, if the cancellation signal fails to stop the tasks, `collectAll` will wait for other tasks to complete.

```cpp
Lazy<int> work1();
Lazy<std::string> work2();
// work1() and work2() both finished, no cancel
auto res = co_await collectAll(work1(), work2());
// work1() and work2() both finished, the latter task will be canceled by CancellationType::terminal
auto res = co_await collectAll<CancellationType::terminal>(work1(), work2());
```

### Transmission and Retrieval of Signal Slots

A `Lazy` can hold a `CancellationSlot`. When we want to bind a cancellation signal to an asynchronous task, we only need to bind the signal at the start of the task using `Lazy<T>::setLazyLocal`. The signal will propagate with `co_await`. Note that the signal can only be bound once.

We can get a pointer to the `CancellationSlot` with `co_await CurrentCancellationSlot{}`. Calling `co_await ForbidCancellation{}` will detach the coroutine call chain from the cancellation signal, preventing subsequent tasks from being aborted. This destructs the corresponding `CancellationSlot` object, and future calls to `co_await CurrentCancellationSlot{}` will return nullptr.

```cpp
Lazy<void> subTask() {
    CancellationSlot* slot = co_await CurrentCancellationSlot{};
    assert(slot != nullptr);
    co_await ForbidCancellation{};
    // slot is now illegal.
    assert(co_await CurrentCancellationSlot{} == nullptr);
    co_return;
}
Lazy<void> Task() {
    CancellationSlot* slot = co_await CurrentCancellationSlot{};
    assert(slot != nullptr);
    co_await subTask();
    assert(co_await CurrentCancellationSlot{} == nullptr);
    co_return;
}
auto signal = CancellationSignal::create();
syncAwait(Task().setLazyLocal(signal.get()).via(ex));
```

### Cancellation-Supported Objects and Functions

In addition to manually checking if the cancellation signal has been triggered, many potentially suspending functions in `async-simple` support cancellation.

The following objects and functions support cancellation. A canceled coroutine may throw a `std::system_error` with an error code of `std::errc::operation_canceled`. Additionally, all functions automatically insert two checkpoints to check if a task has been canceled during suspension/resumption.
1. CollectAny: When canceled, forwards the signal to all sub-tasks and immediately returns with an exception.
2. CollectAll: When canceled, forwards the signal to all sub-tasks, then waits for all sub-tasks to complete before returning normally.
3. Yield/SpinLock: If canceled, an exception is thrown. Currently, tasks queued in the scheduler do not support cancellation.
4. Sleep: Depends on whether the scheduler has overridden the virtual function `void schedule(Func func, Duration dur, uint64_t schedule_info, CancellationSlot *slot = nullptr)` and correctly implemented the cancellation functionality. If this function is not overridden, the default implementation of `async_simple` supports canceling sleep.

The following IO objects and functions do not yet support cancellation and need further improvement.
1. Mutex
2. ConditionVariable
3. SharedMutex
4. Latch
5. Promise/Future
6. CountingSemaphore

### How to Support Cancellation in Custom Awaiters

When implementing custom IO functions, users also need to adapt to support cancellation. `async_simple` provides helper functions like `CancellationSlot::ready()`, `CancellationSlot::suspend()`, and `CancellationSlot::resume()` to simplify user code.

Below is an example that wraps an asynchronous timer into a coroutine-based sleep function with cancellation support:

```cpp
using Duration = std::chrono::milliseconds;
class TimeAwaiter {
public:
    TimeAwaiter(Duration dur, CancellationSlot *slot)
        : _asyncTimer(), _dur(dur), _slot(slot) {}

public:
    bool await_ready() const noexcept { 
      // Check if canceled before suspension (if canceled, call await_resume() immediately)
      return CancellationSlot::ready(_slot); 
    }

    bool await_suspend(std::coroutine_handle<> handle) {
        bool hasnt_canceled = CancellationSlot::suspend(
            _slot, [this](CancellationType) {
                _asyncTimer.cancel();
            });
        if (hasnt_canceled) {
            _asyncTimer.sleep_for(_dur, [handle](auto&&){
                handle.resume();
            });
        } else {
            return true; // resume now
        }
    }

    // Check if canceled after suspension (if canceled, throw exception), if not canceled, clear the slot function.
    void await_resume() { 
      CancellationSlot::resume</*if canceled callback is running, waiting for cancel finished = */true>(_slot); 
    }

    // Helper function to speed up Lazy co_await (it will ignore Lazy's executor)
    auto coAwait(Executor *) {
        return *this;
    }

private:
    AsyncTimer _asyncTimer;
    Duration _dur;
    CancellationSlot *_slot;
};

// Throw exception if canceled.
template <typename Rep, typename Period>
Lazy<void> my_sleep(Duration dur) {
    co_return co_await TimeAwaiter{dur, co_await CurrentCancellationSlot{}};
}
```

Note that the slot function only captures the `this` pointer, so there is no need to extend the timer's lifecycle. This is because when calling `resume`, if it finds that the slot function is being executed, it will synchronously wait for the slot function to complete. This ensures that the slot function runs to completion before the object is destroyed. If you do not want synchronous waiting, you need to extend the `timer`'s lifecycle through reference counting or other mechanisms to prevent concurrent execution of cancellation signals and the end of sleep, causing `TimerAwaiter` to be destroyed before the `timer.cancel()` call.

## Stackful Coroutine Support

Stackful coroutines do not currently have any special support for cancellation, which will be improved in the future.