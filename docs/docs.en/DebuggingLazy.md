The C++20 coroutine frame is meant to be transparent for users by design. In the design of C++20 coroutine,
the users shouldn't care about corouitne frame. The users should only know that the compiler would maintain everything well.
The reason for the decision is that the opaqueness for the usesrs leaves optimization spaces for compilers. Due to coroutine frame is transparent for users, compiler is free to do optimizations, like reducing the size of coroutine frame.

However, the users could believe compiler. But it is necessary for users to know the value in coroutine frame to helping debugging their programs. Due to the implementation of coroutine frame in compilers is not standardized. The method described in the document is not guaranteed to be generalized.

The document is based on Clang (>= 11.0.1) only now.

# Print Coroutine Frame

To print the layout of a coroutine frame, we could use following command to print the layout of the frame for the current coroutine in gdb/lldb:

```
p __coro_frame
```

# Print Promise

Promise is important data structure for each kind of coroutine. We could use following command to print the Promise object for the current frame in gdb/lld:

```
p __promise
```

# Print asynchronous call stack

This section is specified to async_simple. To get the backtrace for stackless coroutine in gdb, we could run:

```
(gdb) source /path/to/async_simple/dbg/LazyStack.py
(gdb) lazy-bt # if you are in stackless coroutine context
(gdb) lazy-bt 0xffffffff # 0xffffffff should be the address of corresponding frame
```

# Print any coroutine frame

This section is specified to async_simple and it requires Clang (>= 15.x).

To print any coroutine frame in gdb, we can run:

```
(gdb) source /path/to/async_simple/dbg/LazyStack.py
(gdb) show-coro-frame 0xffffffff # 0xffffffff should be the address of corresponding frame
```
