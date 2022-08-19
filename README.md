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

The async_simple is a library offering simple, light-weight and easy-to-use 
components to write asynchronous codes. The components offered include the Lazy
(based on C++20 stackless coroutine), the Uthread (based on stackful coroutine)
and the traditional Future/Promise.

# Install Dependencies

The build of async_simple need libaio, googletest and cmake.  Both libaio and googletest
are optional. (Testing before using is highly suggested.)

## Using apt (ubuntu and debian's)

```
# Install libaio
sudo apt install libaio-dev -y

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
```

## Using yum (CentOS and fedora)
```
# Install libaio
sudo yum install libaio-devel -y

sudo yum install cmake -y
# Try to build gtest and gmock from source
```

## Using Homebrew (macOS)

```
brew install cmake
brew install googletest
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

## Install Clang Compiler

Required Compiler: clang (>= 11.0.1) or gcc (>= 10.3)

## Demo example dependency

Demo example depends on standalone asio(https://github.com/chriskohlhoff/asio/tree/master/asio), the commit id:f70f65ae54351c209c3a24704624144bfe8e70a3

# Build
```
$ mkdir build && cd build
# Specify [-DASYNC_SIMPLE_ENABLE_TESTS=OFF] to skip tests.
CXX=clang++ CC=clang cmake ../ -DCMAKE_BUILD_TYPE=[Release|Debug] [-DASYNC_SIMPLE_ENABLE_TESTS=OFF]
# for gcc, use CXX=g++ CC=gcc
make -j4
make test # optional
make install # sudo if required
```

# Get Started

After installing and reading [Lazy](./docs/docs.en/Lazy.md) to get familiar with API, here is a [demo](./docs/docs.en/GetStarted.md) use Lazy to count char in a file.

# Questions

For questions, we suggest to read [docs](./docs/docs.en), [issues](https://github.com/alibaba/async_simple/issues)
and [discussions](https://github.com/alibaba/async_simple/discussions) first.
If there is no satisfying answer, you could file an [issues](https://github.com/alibaba/async_simple/issues)
or start a thread in [discussions](https://github.com/alibaba/async_simple/discussions).
Specifically, for defect report or feature enhancement, it'd be better to file an [issues](https://github.com/alibaba/async_simple/issues). And for how-to-use questions, it'd be better to start a thread in [discussions](https://github.com/alibaba/async_simple/discussions).

# How to Contribute
1. Read the [How to fix issue](./docs/docs.en/HowToFixIssue.md) document firstly.
2. Run tests and `git-clang-format HEAD^` locally for the change.
3. Create a PR, fill in the PR template.
4. Choose one or more reviewers from contributors: (ChuanqiXu9, RainMark, foreverhy, qicosmos).
5. Get approved and merged.

# Performance Monitoring

We could monitor the performance change history in: https://alibaba.github.io/async_simple/benchmark-monitoring/index.html.

# License

async_simple is distributed under the Apache License (Version 2.0)
This product contains various third-party components under other open source licenses.
See the NOTICE file for more information.
