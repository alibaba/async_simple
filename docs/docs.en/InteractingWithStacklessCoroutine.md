We need to offer multiple interfaces for an method implemented in stackless coroutine to make it could be used by different users.
For example, we implement a `Read` method:
```cpp
Lazy<size_t> Read();
```

But the caller of `Read` might be stackless coroutine, stackful coroutine and normal function. Note that the stackless coroutine would be polluting generally. So how should we call it in above cases?

# Stackless coroutine

When the caller is also a stackless coroutine, we could `co_await` it directly:
```cpp
Lazy<size_t> foo() {
    auto val = co_await Read();
    // ...
}
```

# Stackful coroutine

When the caller is in stackful coroutine, we could make a wrapper for `Read` by `await` method:
```cpp
Lazy<size_t> Read();
Future<size_t> ReadAsync() {
    return await(nullptr, Read);
}
void foo() { // in Uthread
    auto val = ReadAsync().get();
    // ...
}
```

## await

`await` method is used for stackful coroutine to wait for stackless coroutine. `await` has two overloads:

### Class member method

```cpp
template <class B, class Fn, class C, class... Ts>
decltype(auto) await(Executor* ex, Fn B::* fn, C* cls, Ts&&... ts);
```

Given the return type of `Fn` is `Lazy<T>`, the return type of `await` would be `T`.

We could use it as:
```cpp
class Foo {
public:
   lazy<T> bar(int a, int b) {}
};
Foo f;
await(ex, &Foo::bar, &f, a, b);
```

`ex` is the executor we specified. It could be null if we don't need an executor.

### Normal Method

```cpp
template <class Fn, class... Ts>
decltype(auto) await(Executor* ex, Fn&& fn, Ts&&... ts);
```

Given the return type of `Fn` is `Lazy<T>`, the return type of `await` would be `T`.

We could use it as:
```cpp
lazy<T> foo(Ts&&...);
await(ex, foo, Ts&&...);
auto lambda = [](Ts&&...) -> lazy<T> {
    // ...
};
await(ex, lambda, Ts&&...);
```

`ex` is the executor we specified. It could be null if we don't need an executor.

# Normal function

The normal function could make a wrapper for `Read` by `syncAwait` method:
```cpp
Lazy<size_t> Read();
size_t ReadSync() {
    return syncAwait(Read());
}
void foo() {
    auto val = ReadSync();
    // ...
}
```

If it need an executor:
```cpp
Lazy<size_t> Read();
size_t ReadSync(Executor* ex) {
    return syncAwait(Read().via(ex));
}
void foo() {
    auto *ex = getExecutor();
    auto val = ReadSync(ex);
    // ...
}
```

# Non-blocking call

No matter the caller is a normal function, a stackful coroutine or a stackless coroutine, the caller could call a stackless coroutine in a non-blocking way by `.start` method.
```cpp
Lazy<size_t> Read();
void ReadCallback(std::function<void(Try<size_t>)> callback) {
    Read().start(callback);
}
void foo() {
    ReadCallback(callback);
    // continue to execute immediately.
}
```
