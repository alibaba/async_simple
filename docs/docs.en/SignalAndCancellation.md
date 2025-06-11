In many cases, we need to cancel some asynchronous tasks associated with another task once it completes. For example, we may want to terminate an IO request when it times out. `async-simple` provides a universal, thread-safe, and efficient asynchronous task cancellation mechanism based on a cooperative signal-slot model and supports structured task cancellation.

## Signal and Slot

`async-simple` offers the following signal types, with each bit representing a specific signal:

```cpp
enum class SignalType : uint64_t {
    none = 0,
    // The lower 32 bits of a signal can only be triggered once.
    terminal = 0b1,                   // The lowest 1 bit represents the termination signal.
                                      // Bits 2-16 are reserved for async-simple.
                                      // Bits 17-32 are for user-defined extensions.
    // The upper 32 bits of a signal can be triggered multiple times (the signal handler can be called multiple times).
                                      // Bits 33-48 are reserved for async-simple.
                                      // Bits 49-64 are for user-defined extensions.
    all = 0xffff'ffff'ffff'ffff,      // Default filtering level (no signal filtering)
};
```

The `Signal` type is used to issue a signal, while `Slot` is used to receive signals. We can create a signal through factory methods and bind multiple `Slot`s to the same `Signal`. When a `Signal` is triggered, all `Slot`s will receive the corresponding signal.

The simplest method is to manually check if a signal has been triggered:

```cpp
// Create a Signal through factory methods
std::shared_ptr<Signal> signal = Signal::create();
std::vector<std::future<void>> works;
for (int i=0;i<10;++i) {
  // Create a Slot for each asynchronous task
  auto slot = std::make_unique<Slot>(signal.get());
  // Execute the task asynchronously
  std::async(std::launch::async, [slot=std::move(slot)] {
      // Manually poll cancellation status
      while (!slot->hasTriggered(SignalType::terminate)) {
        // ...
      }
      return;
  });
}
// ...
// Submit a cancellation signal
signal->emits(SignalType::terminate);
for (auto &e:works)
  e.get();
```

Apart from directly checking the cancellation status, we can register signal handlers in a `Slot` to receive signals. The signature for a signal handler should be `void(SignalType, Signal*)`. The first parameter, `SignalType`, represents the signal type successfully triggered after filtering, and the second parameter is a pointer to the signal.

Note:
1. Signal handlers should not block. When the `emits()` function is called and a signal is triggered, the program will immediately traverse and execute the signal handlers bound to the `Signal`.
2. Be cautious of thread safety: Signal handlers will be executed by the thread calling `emits()`, and signal handlers for signals above the 32nd bit might be executed concurrently by multiple threads.
3. Signal handlers should not hold the signal bound to the slot, as it would cause a memory leak. Users should access the signal via the `Signal*` parameter.

For example, the following code cancels a sleep operation through a signal callback function:

```cpp
// Create a Signal through factory methods
std::shared_ptr<Signal> signal = Signal::create();
std::vector<std::future<void>> works;
for (int i=0;i<10;++i) {
  // Create a slot for each asynchronous task
  auto slot = std::make_unique<Slot>(signal.get());
  // Execute the task asynchronously
  std::async(std::launch::async, [slot=std::move(slot)] {
      std::unique_ptr<std::promise<void>> p;
      auto f = p->get_future();
      bool ok = slot->emplace(SignalType::terminate, // The bit position representing the callback function to be triggered. This enum should have exactly one bit set to 1. If the signal submitted has this bit set to 1, the callback function will be triggered.
          [p=std::move(p)](SignalType, Signal*){
          p->set_value();
      });
      if (ok) { // If the cancellation signal has not been triggered
          f.wait_for(1s*(rand()+1)); // Sleep for a while unless the cancellation signal is triggered.
      }
      slot->clear(); // Clear the callback function
      if (slot->signal()) { // If the slot is bound to a signal
          slot->signal()->emits(SignalType::terminate); // Trigger the cancellation signal.
      }
      return;
  });
}
// ...
// Once the first asynchronous sleep task ends, other tasks will be terminated together.
for (auto &e:works)
  e.get();
```

In the above code, by calling the `Slot`'s `signal()` function, we can get a pointer to the `Signal`. We ensure that the pointer returned by this function is always valid because the lifecycle of the `Signal` is automatically extended until the last bound slot is destructed.

The above code may call the `emit` function multiple times to attempt to trigger the cancellation signal. The `emit` call will return the successfully triggered signal.

```cpp
class Signal
    : public std::enable_shared_from_this<Signal> {
public:
    // Submit a signal (allows submitting multiple signals at once) and returns the successfully triggered signals in the current request. Thread-safe.
    SignalType emits(SignalType state) noexcept;
    // Get the current signal, thread-safe.
    SignalType state() const noexcept;
    // Factory method to create a signal, returns a shared_ptr of the signal. Thread-safe.
    static std::shared_ptr<Signal> create();
};
```

All operations on `Signal` are thread-safe. However, since each asynchronous task should own its own `Slot`, slot objects are not thread-safe. We prohibit the concurrent invocation of the public interface of the same `Slot`.

```cpp
class Slot {
    // Bind `Signal` with `Slot` together. Allows specifying the signal filter level. If the result of the submitted signal type AND filter is zero, the signal will not be triggered.
    Slot(Signal* signal,
                     SignalType filter = SignalType::all);
    // Register a signal handler. Users can register signal handlers multiple times, and the second registration will override the previous handler.
    // SignalType: Represents the bit position to which this signal handler responds. This enum should have exactly one bit set to 1. If the submitted signal has this bit set to 1, the callback function will be triggered.
    // Returns false: Indicates that the cancellation signal has already been triggered.
    template <typename... Args>
    [[nodiscard]] bool emplace(SignalType type, Args&&... args);
    // Clear the signal handler. If returns false, it means the signal handler has already been executed or never been registered.
    bool clear();
    // Filter signals within the specified scope. If the cancellation signal type & filter is zero, the signal type will not be triggered within that scope.
    // Allows nested addition of filters.
    [[nodiscard]] FilterGuard setScopedFilter(SignalType filter);
    // Get the current scope's filter
    SignalType getFilter();
    // Determine if the signal is in a triggered state (returns false if filtered).
    bool hasTriggered(SignalType) const noexcept;
    // This function returns the `Signal` corresponding to the `Slot`. If the signal was triggered before constructing the slot, this function returns nullptr. Otherwise, it always returns a valid `Signal` pointer. This is because the slot holds ownership of the corresponding signal. To extend the signal's lifecycle, you can call signal()->shared_from_this(), or use the signal to start a new coroutine.
    Signal* signal() const noexcept;
};
```

We can chain multiple `Signal`s together. When a particular `Signal` is triggered, the signal will be forwarded to other chained child `Signal`s.

```cpp
std::shared_ptr<Signal> signal = Signal::create();
auto slot = std::make_unique<Slot>(signal.get());
std::shared_ptr<Signal> chainedSignal = Signal::create();
slot->addChainedSignal(chainedSignal);
signal->emits(SignalType::terminate);
assert(chainedSignal->state()==SignalType::terminate);
// The signal will be forwarded to chainedSignal
// However, the signals triggered by chainedSignal will not be forwarded to signal
chainedSignal->emits(static_cast<SignalType>(0b10));
assert(signal->state()!=static_cast<SignalType>(0b10));
```

We can also extend `Signal` to provide more context information to signal handlers:

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
mySignal->myState=1;
mySignal->emits(SignalType::terminate);
```

## Support for Stackless Coroutines

The above slot and signal mechanism is a low-level universal API that adapts to various asynchronous scenarios. `async-simple`'s `Lazy` stackless coroutine library provides a series of advanced encapsulations, allowing users to perform structured concurrent task cancellation without worrying about the details.

### collect functions and structured concurrency

In general user code, we recommend using `collectAny` and `collectAll` to perform structured concurrency task cancellation.

#### collectAny

`collectAny` can concurrently execute multiple coroutines and waits for the first coroutine to return. It will automatically bind these coroutines to a cancellation signal and, upon the first coroutine's completion, send a cancellation signal to terminate any ongoing tasks.

For example, the following code implements a general timeout handling logic using `collectAny`. If `async_read()` does not return within 1 second, `sleep_1s()` will return first and cancel `async_read()`.

```cpp
Lazy<void> sleep_1s();
Lazy<std::string> async_read();
auto res = co_await collectAny<SignalType::terminate>(async_read(),sleep_1s());
if (std::get<res>() == 1) { // timed out!
  // ...
}
else {
  // ...
}
```

`collectAny` supports sending different cancellation signals. By default, it sends the `SignalType::none` signal, which means `async_read()` will not be canceled and will continue to execute. If you want `collectAny` to cancel other tasks upon completion, you can choose to send the `SignalType::terminate` signal.

```cpp
Lazy<void> work1();
Lazy<void> work2();
auto res = co_await collectAny<SignalType::none>(work1(), work2());
if (std::get<res>() == 0) { 
  // work1 finished, work2 will still be working
}
else { 
  // work2 finished, work1 will still be working
}
```

#### collectAll

Similar to `collectAny`, `collectAll` can send a signal when the first task ends. (Default behavior is no signal)

```cpp
Lazy<int> work1();
Lazy<std::string> work2();
// work1(), work2() all finished, no cancel
auto res = co_await collectAll(work1(),work2());
// work1(), work2() all finished, the later work will be canceled by SignalType::terminate
auto res = co_await collectAll<SignalType::terminate>(work1(),work2());
```

Unlike `collectAny`, `collectAll` waits for all coroutines to complete and retrieves their return values. This simplifies the lifecycle of asynchronous tasks, and if the cancellation signal fails to stop a task, `collectAll` will wait for the remaining tasks to finish.

```cpp
http_client client;
// Unlike `collectAny`, we do not need to manually extend the lifecycle of `client` because `collectAll` guarantees that the `http_client::connect` coroutine will return.
auto res = co_await collectAll<SignalType::terminate>(client.connect("localhost:8080"), sleep(1s));
// If no cancellation signal is sent, `collectAll` will continue to execute until the sleep time of one second elapses and the client completes the connection.
auto res = co_await collectAll(client.connect("localhost:8080"), sleep(1s));
```

### Passing and retrieving signals and slots

A `Lazy` can hold a `Slot`. When binding a cancellation signal to an asynchronous task, all that's needed at the beginning of the task is to bind the signal through `Lazy<T>::setLazyLocal`; it will be passed throughout the call chain via `co_await`. Note that a signal can only be bound once.

You can get a pointer to the `Slot` by calling `co_await CurrentSlot{}`, and call `co_await ForbidSignal{}` to unbind the coroutine call chain from the cancellation signal, preventing subsequent tasks from being interrupted. This will destruct the corresponding `Slot` object, and calling `co_await CurrentSlot{}` afterwards will return nullptr.

```cpp
Lazy<void> subTask() {
    Slot* slot = co_await CurrentSlot{};
    assert(slot!=nullptr);
    co_await ForbidSignal{};
    // slot is now invalid.
    assert(co_await CurrentSlot{} == nullptr);
    co_return;
}
Lazy<void> Task() {
    Slot* slot = co_await CurrentSlot{};
    assert(slot!=nullptr);
    co_await subTask();
    assert(co_await CurrentSlot{} == nullptr);
    co_return;
}
auto signal = Signal::create();
syncAwait(Task().setLazyLocal(signal.get()).via(ex));
```

### Objects and functions that support cancellation and signal forwarding

In addition to manually checking for triggered cancellation signals, many potentially suspendable functions in `async-simple` support cancellation operations. Additionally, `collect*` functions support forwarding signals received externally to coroutines initiated by `collect*`.

The following objects and functions support cancellation operations. Coroutines responding to signals may throw the `async_simple::SignalException` exception, and calling `value()` should return the signal type (for cancellation operations, this is `async_simple::terminate`). Also, they will automatically insert two checkpoints to check if the task has been canceled during the suspension/resumption of the coroutine.

1. `CollectAny`: Forwards signals to all subtasks. If a cancellation signal is received, an exception is thrown and it returns immediately.
2. `CollectAll`: Forwards signals to all subtasks. Even if a cancellation signal is received, it waits for all subtasks to complete before normally returning.
3. `Yield/SpinLock`: Throws an exception if canceled. Currently, canceling tasks queued in the scheduler is not supported.
3. `Future`: co_await a future could be canceled. It will return `Try<T>` which contains a exception.
4. `Sleep`: Depends on whether the scheduler overrides the virtual function `void schedule(Func func, Duration dur, uint64_t schedule_info, Slot *slot = nullptr)` and correctly implements the cancellation functionality. If not overridden, the default implementation supports canceling Sleep.

The following IO objects and functions do not yet support cancellation operations and await further improvements:
1. `Mutex`
2. `ConditionVariable`
3. `SharedMutex`
4. `Latch`
6. `CountingSemaphore`

### Supporting cancellation in custom Awaiters

When implementing your own IO functions, it's necessary to adapt and support cancellation functionality. `async-simple` provides `signalHelper{terminate}.hasCanceled()` (for use in the coroutine's `await_ready()`), `signalHelper{terminate}.tryEmplace()` (for use in the coroutine's `await_suspend()`), and `signalHelper{terminate}.checkHasCanceled()` (for use in the coroutine's `await_resume()`) to simplify user code.

Below is an example implementation of a coroutine Sleep function based on an asynchronous timer, which supports interruption by a cancellation signal:

```cpp
using Duration = std::chrono::milliseconds;
class TimeAwaiter {
public:
    TimeAwaiter(Duration dur, Slot *slot)
        : _asyncTimer(...), _dur(dur), _slot(slot) {}

public:
    bool await_ready() const noexcept { 
      // Check if canceled before suspension (if canceled, the coroutine will immediately call await_resume())
      return signalHelper{terminate}.hasCanceled(_slot); 
    }

    void await_suspend(std::coroutine_handle<> handle) {
        _asyncTimer.sleep_for(_dur, [handle](auto&&){
          handle.resume();
        })
        bool hasnt_canceled = signalHelper{terminate}.tryEmplace(
            _slot, [this](SignalType, Signal*) {
                _asyncTimer.cancel();
            });
        if (!hasnt_canceled) { // If canceled
          _asyncTimer.cancel();
        }
    }

    // Check if canceled after suspension (if canceled, throw SignalException). If not canceled, clear slot function.
    void await_resume() { 
      signalHelper{terminate}.checkHasCanceled(_slot); 
    }

    // Helper function to speed up Lazy co_await (it will ignore the lazy's executor)
    auto coAwait(Executor *) {
        return *this;
    }

private:
    AsyncTimer _asyncTimer;
    Duration _dur;
    Slot *_slot;
};

// Throw SignalException if canceled.
template <typename Rep, typename Period>
Lazy<void> my_sleep(Duratio dur) {
    co_return co_await TimeAwaiter{dur,co_await CurrentSlot{}};
}
```

## Support for Stackful Coroutines

Stackful coroutines currently do not have special support for cancellation and await further improvements.