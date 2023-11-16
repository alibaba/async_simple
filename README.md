<p align="center">
<h1 align="center">async_simple</h1>
<h6 align="center">A Simple, Light-Weight Asynchronous C++ Framework</h6>
</p>
<p align="center">
<img alt="license" src="https://img.shields.io/github/license/alibaba/async_simple?style=flat-square">
<img alt="language" src="https://img.shields.io/github/languages/top/alibaba/async_simple?style=flat-square">
<img alt="feature" src="https://img.shields.io/badge/c++20-Coroutines-orange?style=flat-square">
<img alt="last commit" src="https://img.shields.io/github/last-commit/alibaba/async_simple?style=flat-square">
</p>

English | [中文](./README_CN.md)

async_simple is a library offering simple, light-weight and easy-to-use 
components to write asynchronous code. The components offered include Lazy
(based on C++20 stackless coroutine), Uthread (based on stackful coroutine)
and the traditional Future/Promise.

# Quick Experience

We can try async_simple online in [compiler-explorer](compiler-explorer.com): https://compiler-explorer.com/z/Tdaesqsqj . Note that `Uthread` cannot be use in compiler-explorer since it requires installation.

# Install Dependencies

The build of async_simple requires libaio, googletest and cmake.  Both libaio and googletest
are optional. (Testing before using is highly suggested.)

## Using apt (Ubuntu and Debian)

```bash
# Install libaio
sudo apt install libaio-dev -y
# Install cmake
sudo apt install cmake -y
# Install bazel See: https://bazel.build/install/ubuntu
```
- Use `apt` to install gtest and gmock
```bash
sudo apt install -y libgtest-dev libgmock-dev
```

- [Try to build gtest and gmock from source](#Build-Dependencies-From-Source)
```bash
# Install gtest
sudo apt install libgtest-dev -y
sudo apt install cmake -y
cd /usr/src/googletest/gtest
sudo mkdir build && cd build
sudo cmake .. && sudo make install
cd .. && sudo rm -rf build
cd /usr/src/googletest/gmock
sudo mkdir build && cd build
sudo cmake .. && sudo make install
cd .. && sudo rm -rf build

# Install bazel See: https://bazel.build/install/ubuntu
```

## Using yum (CentOS and Fedora)
```bash
# Install libaio
sudo yum install libaio-devel -y
```
- Use `yum` to install gtest, gmock
```
sudo yum install gtest-devel gmock-devel
```
- [Try to build gtest and gmock from source](#Build-Dependencies-From-Source)

## Using Pacman (Arch)
```bash
# Optional
sudo pacman -S libaio
# Use cmake to build project
sudo pacman -S cmake gtest
# Use bazel to build project
sudo pacman -S bazel
```


## Using Homebrew (macOS)

```bash
# Use cmake to build project
brew install cmake
brew install googletest
# Use bazel to build project
brew install bazel
```


## Windows
```powershell
# Install cmake
winget install cmake
# Install google-test
# TODO
# Install bazel See: https://bazel.build/install/windows
```

## Build Dependencies From Source

```
# libaio (optional)
# you can skip this if you install libaio from packages
git clone https://pagure.io/libaio.git
cd libaio
sudo make install
# gmock and gtest
git clone git@github.com:google/googletest.git -b v1.8.x
cd googletest
mkdir build && cd build
cmake .. && sudo make install
```

## Compiler Requirement

Required Compiler: clang (>= 10.0.0) or gcc (>= 10.3) or Apple-clang (>= 14)

Note that we need to add the `-Wno-maybe-uninitialized` option when we use gcc 12 due to a false positive diagnostic message by gcc 12

Note that when using clang 15 it may be necessary to add the `-Wno-unsequenced` option, which is a false positive of clang 15. See https://github.com/llvm/llvm-project/issues/56768 for details.

If you meet any problem about MSVC Compiler Error C4737. Try to add the /EHa option to fix the problem.

## Demo example dependency

Demo example depends on standalone asio(https://github.com/chriskohlhoff/asio/tree/master/asio), the commit id:f70f65ae54351c209c3a24704624144bfe8e70a3

# Build

## cmake
```bash
$ mkdir build && cd build
# Specify [-DASYNC_SIMPLE_ENABLE_TESTS=OFF] to skip tests.
# Specify [-DASYNC_SIMPLE_BUILD_DEMO_EXAMPLE=OFF] to skip build demo example.
# Specify [-DASYNC_SIMPLE_DISABLE_AIO=ON] to skip the build libaio
CXX=clang++ CC=clang cmake ../ -DCMAKE_BUILD_TYPE=[Release|Debug] [-DASYNC_SIMPLE_ENABLE_TESTS=OFF] [-DASYNC_SIMPLE_BUILD_DEMO_EXAMPLE=OFF] [-DASYNC_SIMPLE_DISABLE_AIO=ON]
# for gcc, use CXX=g++ CC=gcc
make -j4
make test # optional
make install # sudo if required
```

Conan is also supported. You can install async_simple to conan local cache.
```
mkdir build && cd build
conan create ..
```

## bazel
```bash
# Specify [--define=ASYNC_SIMPLE_DISABLE_AIO=true] to skip the build libaio
# Example bazel build --define=ASYNC_SIMPLE_DISABLE_AIO=true ...
bazel build ...                      # compile all target
bazel build ...:all                  # compile all target
bazel build ...:*                    # compile all target
bazel build -- ... -benchmarks/...   # compile all target except those beneath `benchmarks`
bazel test ...                       # compile and execute tests
# Specify compile a target
# Format: bazel [build|test|run] [directory name]:[binary name]
# Example
bazel build :async_simple           # only compile libasync_simple
bazel run benchmarks:benchmarking   # compile and run benchmark
bazel test async_simple/coro/test:async_simple_coro_test
# Use clang toolchain
bazel build --action_env=CXX=clang++ --action_env=CC=clang ...
# Add compile option 
bazel build --copt='-O0' --copt='-ggdb' ...
```
- See [this](https://bazel.build/run/build) get more infomation
- ` ...` Indicates recursively scan all targets, recognized as `../..` in `oh-my-zsh`, can be replaced by other `shell` or `bash -c 'commond'` to run, such as `bash -c 'bazel build' ...` or use `bazel build ...:all`
- Use `async_simple` as a dependency, see also [bazel support](bazel/support/README.md)

# Docker Compile Environment
```
git clone https://github.com/alibaba/async_simple.git
cd async_simple/docker/(ubuntu|centos7|rockylinux)
docker build . --no-cache -t async_simple:1.0
docker run -it --name async_simple async_simple:1.0 /bin/bash
```

# Get Started

Our documents are hosted by GitHub Pages, [go to Get Started](https://alibaba.github.io/async_simple/docs.en/GetStarted.html).

After installing and reading [Lazy](./docs/docs.en/Lazy.md) to get familiar with API, here is a [demo](./docs/docs.en/GetStarted.md) use Lazy to count char in a file.

# Performance

We also give a [Quantitative Analysis Report](docs/docs.en/QuantitativeAnalysisReportOfCoroutinePerformance.md) of
Lazy (based on C++20 stackless coroutine) and Uthread (based on stackful coroutine).

# C++20 Modules Support

We have **experimental** support for C++20 Modules in `modules/async_simple.cppm`. 
We can build the `async_simple` module by `xmake` and `cmake`.
We can find the related usage in `CountChar`, `ReadFiles`, `LazyTest.cpp` and `FutureTest.cpp`.

We need clang (>= d18806e6733 or simply clang 16) to build the `async_simple` module.
It is only tested for libstdc++10.3. Due to the current support status for C++20, it won't be a surprise if the compilation fails in higher versions of STL.

We can build the `async_simple` module with xmake (>= 0eccc6e) using the commands:

```
xmake
```

We can build the `async_simple` module with cmake (>= d18806e673 or cmake3.26) using the following commands:

```
mkdir build_modules && cd build_modules
CC=clang CXX=clang++ cmake .. -DCMAKE_BUILD_TYPE=Release -DASYNC_SIMPLE_BUILD_MODULES=ON -GNinja
ninja
```

**Note** that the `async_simple` module in the main branch is actually a named module's wrapper for headers for compatability. We can find the practical usage of C++20 Modules in https://github.com/alibaba/async_simple/tree/CXX20Modules, which contains the support for xmake and cmake as well.

# Questions

For questions, we suggest to read [docs](./docs/docs.en), [issues](https://github.com/alibaba/async_simple/issues)
and [discussions](https://github.com/alibaba/async_simple/discussions) first.
If there is no satisfying answer, you could file an [issues](https://github.com/alibaba/async_simple/issues)
or start a thread in [discussions](https://github.com/alibaba/async_simple/discussions).
Specifically, for defect report or feature enhancement, it'd be better to file an [issues](https://github.com/alibaba/async_simple/issues). And for how-to-use questions, it'd be better to start a thread in [discussions](https://github.com/alibaba/async_simple/discussions).

# How to Contribute
1. Read the [How to fix issue](./docs/docs.en/HowToFixIssue.md) document firstly.
2. Run tests and `git-clang-format HEAD^` locally for the change. Note that the  version of clang-format in CI is clang-format 14. So that it is possible your local 
format result is inconsistency with the format result in the CI. In the case, you need to install the new clang-format or adopt the suggested change by hand. In case the format result is not good, it is OK to accept the PR temporarily and file an issue for the clang-formt.
3. Create a PR, fill in the PR template.
4. Choose one or more reviewers from contributors: (e.g., ChuanqiXu9, RainMark, foreverhy, qicosmos).
5. Get approved and merged.

# Contact us
Please scan the following QR code of DingTalk to contact us.  
<center>
<img src="./docs/docs.cn/images/ding_talk_group.png" alt="dingtalk" width="200" height="200" align="bottom" />
</center>

# License

async_simple is distributed under the Apache License (Version 2.0)
This product contains various third-party components under other open source licenses.
See the NOTICE file for more information.
