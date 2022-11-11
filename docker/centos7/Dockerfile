FROM centos:7.9.2009

ENV LANG=en_US.UTF-8
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

WORKDIR /root
RUN yum update --nogpgcheck -y \
    && yum install --nogpgcheck -y epel-release centos-release-scl \
    && yum install --nogpgcheck -y devtoolset-11-gcc-c++ libaio-devel.x86_64 git cmake3

RUN echo "source /opt/rh/devtoolset-11/enable" >> .bashrc

ENV ASYNC_SIMPLE_ROOT="/root/async_simple"
RUN git clone https://github.com/alibaba/async_simple.git $ASYNC_SIMPLE_ROOT
WORKDIR $ASYNC_SIMPLE_ROOT
