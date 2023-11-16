FROM centos:7.9.2009

ENV LANG=en_US.UTF-8
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

WORKDIR /root

RUN yum update --nogpgcheck -y \
    && yum install --nogpgcheck -y epel-release centos-release-scl \
    && yum install --nogpgcheck -y devtoolset-11-gcc-c++ libaio-devel.x86_64 git cmake3

RUN echo "source /opt/rh/devtoolset-11/enable" >> .bashrc

RUN git clone https://github.com/alibaba/async_simple.git \
    && source /opt/rh/devtoolset-11/enable \
    && cd async_simple \
    && mkdir build && cd build \
    && cmake3 .. -DCMAKE_BUILD_TYPE=Release -DASYNC_SIMPLE_ENABLE_TESTS=OFF \
    && make -j 9

WORKDIR /root/async_simple
