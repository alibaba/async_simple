The branch is used to test the compiler support of C++20 modules, explore the
practice with C++20 modules and compare the coding style between header based
and named module based.

# Structure

The modules code lives in `*_module` direcotories. The third_party_module/stdmodules
provides an implementation for std modules. The third_party_module/asio provides an
implementation for a named wrapper for the asio library. Both the std modules and async_simple
modules are wrappers for header in fact. The async_simple_module provides
async_simple module, which rewrites the whole implementation by named modules. We could
find the practice of named modules in it. The async_simple_module/test tests that the runtime
behavior is still fine. The demo_example_module provides some demo examples 
(including some sync/async http client/servers) by using async_simple module, std module and
asio module.

# Build System

Due to the lack of modules support for C++20 modules in linux environments,
the build script is written in Makefile.

# ToolChain requirement

Compiler: clang15.x
STL Libraries: libstdc++10.3
Other dependencies: libaio, gtest, gmock

# How to run

```
make -j
```
