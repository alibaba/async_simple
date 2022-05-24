在 C++20 协程的设计中，协程帧对于程序员来说应该是无感知的。这种设计使得编译器有更多空间对协程帧进行优化。然而程序员在调试时常常需要查看程序的内存布局。由于协程帧不是标准的一部分，且各个编译器的实现有差异。此文档中所描述的方法只对 Clang (>= 11.0.1) 中生效。

# Print Coroutine Frame

我们可以在 gdb/lldb 中通过以下命令打印协程帧的内存：

```
p __coro_frame
```

# Print Promise

我们可以在 gdb/lldb 中通过以下命令打印当前协程 Promise 对象的内容。

```
p __promise
```

# Print asynchronous call stack

此功能由 dbg/LazyStack.py 脚本实现，只对 async_simple 有效。

我们可以通过以下命令打印出无栈协程的异步调用栈。

```
(gdb) source /path/to/async_simple/dbg/LazyStack.py
(gdb) lazy-bt # if you are in stackless coroutine context
(gdb) lazy-bt 0xffffffff # 0xffffffff should be the address of corresponding frame
```

# Print any coroutine frame

本节所描述的方法只对 Clang (>= 15.x) 生效。此功能由 dbg/LazyStack.py 脚本实现，只对 async_simple 有效。

`p __coro_frame` 可以打印当前协程帧中的内容，但无法打印非当前协程帧中的内容。
我们可以使用 dbg/LazyStack.py 中实现的 `show-coro-frame` 命令来打印任意协程的内容。

```
(gdb) source /path/to/async_simple/dbg/LazyStack.py
(gdb) show-coro-frame 0xffffffff # 0xffffffff should be the address of corresponding frame
```
