# Try

Try is a value which could be exception potentially. For a object `t`, with the type `Try<T>`, we could consider `t` is an exception, or value with type `T`, or nothing.

In async_simple, Try represents a result which could be potentially exception. For example, both the argument of the callback in `Lazy::start` and the argument of the callback in `Future::thenTry` are Try.

The idea and implementation of Try comes from Folly.

Try has three state: Nothing, Exception and Value.

The state of a Try object constructed by the default constructor is Nothing.
We could use `.available()` method to query if a Try object is Nothing.
```cpp
void foo() {
    Try<int> t;
    assert(!t.available());
    // ...
}
```

We could use `hasError()` to judge if there is Exception.

```cpp
void foo() {
    Try<int> t(std::make_exception_ptr(std::runtime_error("")));
    assert(t.hasError());
    // ...
}
```


We could use `getException()` and `value()` to get the exception and value from Try. For example:
```cpp
void foo() {
    Try<int> t(std::make_exception_ptr(std::runtime_error("")));
    std::exception_ptr e = t.getException()
    // ...
    Try<int> t2(5);
    int res = t2.value();
}
```

Note that it would throw exception if we use `value()` to extract the value from a Try object whose state is Exception:
```cpp
void foo() {
    Try<int> t(std::make_exception_ptr(std::runtime_error("")));
    int res = t.value(); // Throw Exception
}
```
