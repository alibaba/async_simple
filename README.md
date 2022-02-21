The async_simple is a library offering simple, light-weight and easy-to-use 
components to write asynchronous codes. The components offered include the Lazy
(based on C++20 stackless coroutine), the Uthread (based on stackful coroutine)
and the traditional Future/Promise.

# Install Dependencies

## Using apt (ubuntu and debian's)

```
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
sudo yum install libaio-devel -y
sudo yum install cmake -y
# Try to build gtest and gmock from source
```

## Build Dependencies From Source

```
# libaio
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

Required Compiler: clang (>= 11.0.1)

## Demo example dependency

Demo example depends on standalone asio(https://github.com/chriskohlhoff/asio/tree/master/asio), the commit id:f70f65ae54351c209c3a24704624144bfe8e70a3

# Build
```
$ mkdir build && cd build
CXX=clang++ CC=clang cmake ../ -DCMAKE_BUILD_TYPE=[Release|Debug]
make -j4
make test # optional
make install # sudo if required
```

# Get Started

After installing and reading [Lazy](./docs/docs.en/Lazy.md) to get familiar with API, here is a [demo](./docs/docs.en/GetStarted.md) use Lazy to count char in a file.

# License

async_simple is distributed under the Apache License (Version 2.0)
This product contains various third-party components under other open source licenses.
See the NOTICE file for more information.
