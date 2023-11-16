FROM ubuntu:22.04

WORKDIR /root

RUN apt-get update -y \
    && apt-get install -y cmake git clang-13 libaio-dev libgtest-dev libgmock-dev gcc-12 g++-12 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100 \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-13 100 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-13 100

RUN git clone https://github.com/alibaba/async_simple.git
RUN cd async_simple \
    && mkdir clang_build && cd clang_build \
    && CXX=clang++ CC=clang cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j 9 && ctest

RUN cd async_simple \
    && mkdir gcc_build && cd gcc_build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j 9 && ctest

WORKDIR /root/async_simple
