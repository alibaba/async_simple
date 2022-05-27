# Try

Try 表示一个可能异常的值。具体地，`Try<T>` 类型的对象 `t` 可以理解为要么是异常，要么是类型 `T` 的值，要么两者都不是。

Try 在 async_simple 中主要用于表示可能出现异常的结果，例如 `Lazy::start` 中 callback 的参数、或者 `Future::thenTry` 中 callback 的参数。

Try 的 idea 和实现来自 Facebook 的 Folly 库。

Try 有三种状态：无状态、有异常、有值。

使用默认构造函数构造的 Try 的状态为无状态，可以使用 `.available()` 接口检测。例如：
```C++
void foo() {
    Try<int> t;
    assert(!t.available());
    // ...
}
```

同时可以使用 `hasError()` 判断是否存在异常。

```C++
void foo() {
    Try<int> t(std::make_exception_ptr(std::runtime_error("")));
    assert(t.hasError());
    // ...
}
```

同时可以使用 `getException()` 与 `value()` 获取 Try 中的异常与值。例如:
```C++
void foo() {
    Try<int> t(std::make_exception_ptr(std::runtime_error("")));
    std::exception_ptr e = t.getException()
    // ...
    Try<int> t2(5);
    int res = t2.value();
}
```

同时需要注意，如果在 Try 对象处于有异常状态时使用 `value()` 取值，会直接抛出异常：
```C++
void foo() {
    Try<int> t(std::make_exception_ptr(std::runtime_error("")));
    int res = t.value(); // 抛出异常！
}
```
