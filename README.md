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

# ToolChain requirement

Compiler: clang15.x or higher versions
STL Libraries: libstdc++10.3, libc++15.x or higher versions.
Other dependencies: gtest, gmock

# Build System

Now we can use cmake (higher or equal than d18806e67336d96a9a22b860), xmake (higher or equal than 0eccc6e) and Makefile to build this demo.

If you choose cmake or xmake to build the project, the compiler should be clang16 (or higher than 64287d69827c).

# How to run

## Make
```
make -j
```

## XMake

```
xmake && xmake -r
```

## CMake

```
mkdir build_cmake && cd build_cmake
cmake .. -GNinja
ninja
ninja test
```

This method will download gtest automatically. In case it is problematic to download gtest, we can specify the location
of gtest by cmake variables: `-DGMOCK_INCLUDE_DIR=<path-to-headers of gtest> -DGTEST_INCLUDE_DIR=<path-to-headers of mock> -DGTEST_LIBRARIES=<path-to-library-of-gtest>  -DGMOCK_LIBRARIES=<path-to-library-of-gmock>`.
