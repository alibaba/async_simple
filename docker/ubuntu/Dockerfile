FROM ubuntu:22.04

ENV TZ=UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
ENV LANG=en_US.UTF-8 \
    LANGUAGE=en_US:en \
    LC_ALL=en_US.UTF-8

WORKDIR /root

RUN apt-get update -y \
    && apt-get install -y cmake git clang-13 libaio-dev libgtest-dev libgmock-dev gcc-11 g++-11

ENV ASYNC_SIMPLE_ROOT="/root/async_simple"
RUN git clone https://github.com/alibaba/async_simple.git $ASYNC_SIMPLE_ROOT
WORKDIR $ASYNC_SIMPLE_ROOT
