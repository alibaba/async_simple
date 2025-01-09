We often need to cancel other asynchronous tasks related to a specific asynchronous task once it finishes. For example, we might want an IO request to terminate promptly if it times out. `async-simple`, based on a collaborative signal-slot model, provides a general, thread-safe, and efficient mechanism for canceling asynchronous tasks. It also supports structured task cancellation.

## Signal and Slot

`async-simple` provides the following signal types:

```cpp
enum class SignalType : uint64_t {
    none = 0,
    // The lower 32 bits can only trigger once.
    terminal = 0b1,                   // The lowest bit represents the termination signal.
                                      // 2-16 bits are reserved for async-simple.
                                      // 17-32 bits are for user-defined extensions.
    // The upper 32 bits can be triggered multiple times (the signal handler can be called multiple times).
                                      // 33-48 bits are reserved for async-simple.
                                      // 49-64 bits are for user-defined extensions.
    all = 0xffff'ffff'ffff'ffff,      // Default filtering level (does not filter any signals)
};
```

The `Signal` type is used to initiate a signal, while the `Slot` is used to receive signals. We can create a signal using a factory method and bind multiple slots to the same signal. When the signal is triggered, all slots will receive the signal.

The simplest approach is to manually check if a signal has been triggered:

```cpp
// Create a signal using a factory method
std::shared_ptr<Signal> signal = Signal::create();
std::vector<std::future<void>> works;
for (int i = 0; i < 10; ++i) {
  // Create a slot for each asynchronous task
  auto slot = std::make_unique<Slot>(signal.get());
  // Asynchronously execute the task
  std::async(std::launch::async, [slot = std::move(slot)] {
      // Manually poll the cancellation status
      while (!slot->hasTriggered(SignalType::terminate)) {
        // ...
      }
      return;
  });
}
// ...
// Submit a cancellation signal
signal->emit(SignalType::terminate);
for (auto &e: works)
  e.get();
```

In addition to directly checking the cancellation status, we can register signal handling functions in the slot to receive signals. The signature of a signal handling function should be `void(SignalType, Signal*)`. The first parameter `SignalType` represents the type of signal successfully triggered after filtering, and the second parameter is a pointer to the signal.

Note:
1. Signal handling functions should not block. When the `emit()` function is called and a signal is triggered, the program traverses all signal handling functions bound to the signal and executes them immediately.
2. Pay attention to thread safety: the signal handling function will be executed by the thread calling `emit()`, and handling functions for signals higher than 32 might be executed concurrently by multiple threads.
3. Signal handling functions should not hold on to the signal bound to the slot, as it may cause memory leaks of the signal. Access the signal through the `Signal*` parameter instead.

For example, the following code uses a signal callback function to cancel a sleep.

```cpp
// Create a signal using a factory method
std::shared_ptr<Signal> signal = Signal::create();
std::vector<std::future<void>> works;
for (int i = 0; i < 10; ++i) {
  // Create a slot for each asynchronous task
  auto slot = std::make_unique<Slot>(signal.get());
  // Asynchronously execute the task
  std::async(std::launch::async, [slot = std::move(slot)] {
      std::unique_ptr<std::promise<void>> p;
      auto f = p->get_future();
      bool ok = slot->emplace(SignalType::terminate, // Represents the bit that triggers the callback function, this enum can have only one bit set to 1, if the signal bit is 1, the callback function will be triggered.
          [p = std::move(p)](SignalType, Signal*) {
          p->set_value();
      });
      if (ok) { // If the cancellation signal has not been triggered
          f.wait_for(1s * (rand() + 1)); // Unless the cancellation signal is triggered, continue sleeping for some time.
      }
      slot->clear(); // Clear the callback function
      if (slot->signal()) { // If the slot is bound to a signal
          slot->signal()->emit(SignalType::terminate); // Trigger the cancellation signal.
      }
      return;
  });
}
// ...
// When the first asynchronous timed task completes, other tasks will also end.
for (auto &e: works)
  e.get();
```

In the above code, we can obtain a pointer to the signal by calling the slot's `signal()` function. We ensure that the pointer returned by this function is always legal, as the signal's lifecycle is automatically extended until the last bound slot is destructed.

The code above may call the `emit()` function multiple times to attempt to trigger the cancellation signal; `emit()` will return the signal triggered successfully each time.

```cpp
class Signal
    : public std::enable_shared_from_this<Signal> {
public:
    // Submit a signal and return this request's successfully triggered signals, thread-safe.
    SignalType emit(SignalType state) noexcept;
    // Get the current cancellation signal, thread-safe.
    SignalType state() const noexcept;
    // The unique method to create a signal, returns a shared_ptr.
    static std::shared_ptr<Signal> create();
};
```

All operations of `Signal` are thread-safe. However, since each asynchronous task should hold its own `Slot`, the slot object is not thread-safe. Users are prohibited from calling the public interface of the same `Slot` concurrently.

```cpp
class Slot {
    // Bind the signal to the slot. A signal filter level can be specified; if the cancellation signal type & filter is 0, the signal will not trigger.
    Slot(Signal* signal,
                     SignalType filter = SignalType::all);
    // Register a signal handling function. Users can register the signal handling function multiple times; the second registration overwrites the previous one.
    // SignalType: Represents the bit that this signal handling function will respond to. This enum can have only one bit set to 1; if the signal bit is 1, the callback function will be triggered.
    // Returns false: The cancellation signal has already been triggered.
    template <typename... Args>
    [[nodiscard]] bool emplace(SignalType type, Args&&... args);
    // Clears the signal handling function. If false is returned, it means the signal handling function has been executed, or the signal has not been registered.
    bool clear();
    // Filter signals within the specified scope; if the cancellation signal type & filter is 0, the signal type will not be triggered in this scope.
    // Nested filter addition is allowed.
    [[nodiscard]] FilterGuard setScopedFilter(SignalType filter);
    // Get the filter of the current scope
    SignalType getFilter();
    // Determines whether a signal is in the triggered state (will return false if filtered).
    bool hasTriggered(SignalType) const noexcept;
    // This function returns the signal corresponding to the slot. If the cancellation signal was triggered before constructing the slot, this function returns nullptr. Otherwise, it always returns a valid signal pointer. This is because the slot owns the corresponding signal. To extend the lifespan of the signal, call signal()->shared_from_this(), or you can use the signal to start a new coroutine.
    Signal* signal() const noexcept;
};
```

We can also provide more contextual information for signal handling functions by inheriting the signal:

```cpp
class MySignal : public Signal {
    using Signal::Signal;
  public:
    std::atomic<size_t> myState;  
};

auto mySignal = Signal::create<MySignal>();
auto slot = std::make_unique<Slot>(mySignal->get());
slot->emplace([](SignalType type, Signal* signal) {
  auto mySignal = dynamic_cast<MySignal*>(signal);
  std::cout << "myState:" << mySignal->myState << std::endl;
});
mySignal->myState = 1;
mySignal->emit(SignalType::terminate);
```

## Stackless Coroutine Support

The above slot and signal are relatively low-level general APIs, adaptable to various asynchronous scenarios. In the `Lazy` stackless coroutine library of `async-simple`, a series of high-level encapsulations are provided, allowing users to achieve structured concurrent task cancellation without worrying about details.

### collect Functions and Structured Concurrency

In general user code, we recommend using `collectAny` and `collectAll` to implement structured concurrent task cancellation.

#### collectAny

`collectAny` can execute multiple coroutines concurrently and wait for the first coroutine to return. `collectAny` will automatically bind these coroutines to a cancellation signal and send a cancellation signal to other coroutines that have not completed execution when the first coroutine finishes, thus ending these tasks.

For example, here is a code that uses `collectAny` to implement general timeout handling logic. If `async_read()` does not return within 1 second, `sleep_1s()` will return first and cancel `async_read()`.

```cpp
Lazy<void> sleep_1s();
Lazy<std::string> async_read();
auto res = co_await collectAny<SignalType::terminate>(async_read(), sleep_1s());
if (std::get<res>() == 1) { // timed out!
  // ...
}
else {
  // ...
}
```

`collectAny` supports users sending different cancellation signals. By default, the cancellation signal sent is `SignalType::none`, which means the `async_read()` task will not be canceled and will continue execution if it completes later. To cancel other tasks when `collectAny` finishes, you can choose to send the signal `SignalType::terminate`.

```cpp
Lazy<void> work1();
Lazy<void> work2();
auto res = co_await collectAny<SignalType::none>(async_read(), sleep_1s());
if (std::get<res>() == 0) { 
  // work1 finished, work2 will still working
}
else { 
  // work2 finished, work1 will still working
}
```

#### collectAll

Similar to `collectAny`, `collectAll` can also send a signal when the first task ends (it does not send a signal by default).

```cpp
Lazy<int> work1();
Lazy<std::string> work2();
// work1(), work2() all finished, no cancel
auto res = co_await collectAll(work1(), work2());
// work1(), work2() all finished, the later work will be canceled by SignalType::terminate
auto res = co_await collectAll<SignalType::terminate>(work1(), work2());
```

Unlike `collectAny`, `collectAll` waits for coroutines to complete execution and retrieves their return values. This simplifies the lifecycle of asynchronous tasks on one hand, and on the other, if the cancellation signal fails to terminate the tasks, `collectAll` ensures all tasks finish executing.

```cpp
http_client client;
// Unlike collectAny, we don't need to extend the lifecycle of the client by reference counting, as collectAll guarantees the http_client::connect coroutine will return.
auto res = co_await collectAll<SignalType::terminate>(client.connect("localhost:8080"), sleep(1s));
// If no cancellation signal is sent, collectAll will execute until 1 second has passed and the client completes the connection.
auto res = co_await collectAll(client.connect("localhost:8080"), sleep(1s));
```

### Passing and Retrieving Signals and Slots

A `Lazy` can hold a `Slot`. When you want to bind a cancellation signal to an asynchronous task, just bind the signal at the start of the task via `Lazy<T>::setLazyLocal`, and it will be passed along through `co_await`. Note that a signal can only be bound once.

You can get a pointer to the `Slot` by calling `co_await CurrentSlot{}`, and calling `co_await ForbidSignal{}` will unbind the coroutine call chain from the cancellation signal, thus preventing subsequent tasks from being terminated. This action will destruct the corresponding `Slot` object, and subsequent calls to `co_await CurrentSlot{}` will return nullptr.

```cpp
Lazy<void> subTask() {
    Slot* slot = co_await CurrentSlot{};
    assert(slot != nullptr);
    co_await ForbidSignal{};
    // The slot is illegal now.
    assert(co_await CurrentSlot{} == nullptr);
    co_return;
}

Lazy<void> Task() {
    Slot* slot = co_await CurrentSlot{};
    assert(slot != nullptr);
    co_await subTask();
    assert(co_await CurrentSlot{} == nullptr);
    co_return;
}

auto signal = Signal::create();
syncAwait(Task().setLazyLocal(signal.get()).via(ex));
```

### Supported Cancelable Objects and Functions

In addition to manually checking if a cancellation signal has been triggered, many potentially suspended functions in `async-simple` support cancellation operations.

The following objects and functions support cancellation operations, where the coroutine responding to the signal may throw the `async_simple::SignalException` exception, and calling `value()` should return the signal type (for cancellation operations, it is `async_simple::terminate`).

Additionally, the following IO functions automatically insert two checkpoints to determine whether the task has been canceled while suspending/resuming the coroutine.

1. `CollectAny`: When canceled, it forwards the signal to all subtasks and throws an exception immediately.
2. `CollectAll`: When canceled, it forwards the signal to all subtasks, then waits for all subtasks to complete before returning normally.
3. `Yield/SpinLock`: If canceled, it throws an exception. Currently, it does not support canceling tasks queued in the scheduler.
4. `Sleep`: This depends on whether the scheduler overrides the virtual function `void schedule(Func func, Duration dur, uint64_t schedule_info, Slot *slot = nullptr)` and correctly implements the cancellation feature. If the function is not overridden, the default implementation supports canceling sleep.

The following IO objects and functions do not yet support cancellation operations and need to be improved in the future:
1. `Mutex`
2. `ConditionVariable`
3. `SharedMutex`
4. `Latch`
5. `Promise/Future`
6. `CountingSemaphore`

### How Custom Awaiter Supports Cancellation

When implementing custom IO functions, users also need to adapt to support cancellation functionality. `async-simple` provides helper functions like `signal_await{terminate}.ready()`, `signal_await{terminate}.suspend()`, and `signal_await{terminate}.resume()` to simplify user code.

Below is an example demonstrating how to wrap an asynchronous timer as a coroutine-based Sleep function with cancellation support:

```cpp
using Duration = std::chrono::milliseconds;

class TimeAwaiter {
public:
    TimeAwaiter(Duration dur, Slot *slot)
        : _asyncTimer(...), _dur(dur), _slot(slot) {}

public:
    bool await_ready() const noexcept { 
      // check if canceled before suspend (if canceled, call await_resume() immediately)
      return signal_await{terminate}.ready(_slot); 
    }

    void await_suspend(std::coroutine_handle<> handle) {
        _asyncTimer.sleep_for(_dur, [](auto&&) {
          handle.resume();
        })
        bool hasnt_canceled = signal_await{terminate}.suspend(
            slot, [this](SignalType, Signal*) {
                _asyncTimer.cancel();
            });
        if (!hasnt_canceled) { // has canceled
          _asyncTimer.cancel();
        }
    }

    // check if canceled after suspend (if canceled, throw exception), if not canceled, clear slot function.
    void await_resume() { 
      signal_await{terminate}.resume(_slot); 
    }

    // helper function to speed-up Lazy co_await (it will ignore lazy's executor)
    auto coAwait(Executor *) {
        return *this;
    }

private:
    AsyncTimer _asyncTimer;
    Duration _dur;
    Slot *_slot;
};

// throw exception if canceled.
template <typename Rep, typename Period>
Lazy<void> my_sleep(Duration dur) {
    co_return co_await TimeAwaiter{dur, co_await CurrentSlot{}};
}
```

## Stackful Coroutine Support

Stackful coroutines do not yet have special support for cancellation and need to be improved in the future.
