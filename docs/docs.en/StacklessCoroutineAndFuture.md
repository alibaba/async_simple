This documents tries to compare the styles of stackless coroutine and Future/Promise by a simple asynchoronous demo. We could find the example codes in `async_simple/demo_example/ReadFiles`.

Readfiles is a demo program to calculate the number of a specific character in multiple files. Here are three methods to implement it:
- Traditional synchronous style.
- Future/Promise based.
- Stackless coroutine (Lazy) based.

## Traditional synchronous style

```cpp
uint64_t CountCharInFiles(const std::vector<FileName> &Files, char c) {
    uint64_t ReadSize = 0;
    for (const auto &File : Files)
        ReadSize += CountCharInFile(File, c);
    return ReadSize;
}
```

It is very straight forward, we could only need to traverse it.

## Future/Promise based

Althought the tranditional synchronous style is very simple, it might not be sufficent when the number of files is very large or the contents of one file is very large or the file is stored in remote storage. In this case, programmers might try to optimize it by asynchronous style. And Future/Promise is a classic asynchronous component.

```cpp
Future<uint64_t> CountCharInFilesAsync(const std::vector<FileName> &Files, char c) {
    // std::shared_ptr is necessary here. Since ReadSize may be used after
    // CountCharInFilesAsync function ends.
    auto ReadSize = std::make_shared<uint64_t>(0);
    // We need to introduce another API `do_for_each` here.
    return do_for_each(std::move(Files), [ReadSize, c](auto &&File){
        return CountCharInFileAsyncImpl(File, c).thenValue([ReadSize](auto &&Size) {
            *ReadSize += Size;
            return makeReadyFuture(Unit());
        });
    }).thenTry([ReadSize] (auto&&) { return makeReadyFuture<uint64_t>(*ReadSize); });;
}
```

The example above implements similar functionality. We need to introduce `std::shared_ptr` and new API `do_for_each`, and the structure becomes much more complex.

## Stackless coroutine (Lazy) based

Programmers could write asynchronous code in synchronous way with stackless coroutine.

```cpp
Lazy<uint64_t> CountCharInFilesCoro(const std::vector<FileName> &Files, char c) {
    uint64_t ReadSize = 0;
    for (const auto &File : Files)
        ReadSize += co_await CountCharFileCoroImpl(File, c);
    co_return ReadSize;
}
```

We could found the style of coroutine based and tranditional synchronous style is basically similar. Also the performance of the coroutine based version would be slightly better than the Future/Promsie based version. On the one hand, we don't need `std::shared_ptr` in coroutine based version. On the other hand, the code in coroutine based version would be less and more continuous, which is more convienient for compiler to do optimization.
