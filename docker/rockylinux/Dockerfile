FROM rockylinux:9.2

WORKDIR /root

RUN dnf update -y \
    && dnf install -y cmake.x86_64 git.x86_64 clang.x86_64 libaio-devel.x86_64 gcc.x86_64 gcc-c++.x86_64

RUN git clone https://github.com/alibaba/async_simple.git
RUN cd async_simple \
    && mkdir clang_build && cd clang_build \
    && CXX=clang++ CC=clang cmake .. -DCMAKE_BUILD_TYPE=Release -DASYNC_SIMPLE_ENABLE_TESTS=OFF \
    && make -j 9

RUN cd async_simple \
    && mkdir gcc_build && cd gcc_build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j 9 && ctest

WORKDIR /root/async_simple
