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

中文 | [English](./README.md)

async\_simple是阿里巴巴开源的轻量级C++异步框架。提供了基于C++20无栈协程（Lazy），有栈协程（Uthread）以及Future/Promise等异步组件。async\_simple诞生于阿里巴巴智能引擎事业部，目前广泛应用于图计算引擎、时序数据库、搜索引擎等在线系统。连续两年经历天猫双十一磨砺，承担了亿级别流量洪峰，具备非常强劲的性能和可靠的稳定性。

# 准备环境

async\_simple 涉及 C++20 协程，对编译器版本有较高要求。需要 clang11 或 gcc10 及其以上版本。编译运行测试需要 gtest。需要 libaio 的功能的需要依赖 libaio。

## Debian/Ubuntu系列

- 安装clang11及其以上版本。安装方法见：[APT Packages](https://apt.llvm.org/)
- 安装cmake。
- 如需使用 libaio 功能，需安装 libaio。

```bash
sudo apt install cmake libaio-dev -y
```

- 安装gtest、gmock。用apt命令安装源代码后再编译安装。

```bash
sudo apt install libgtest-dev -y
# gtest
cd /usr/src/googletest/gtest
sudo mkdir build && cd build
sudo cmake .. && sudo make install
cd .. && sudo rm -rf build
# gmock
cd /usr/src/googletest/gmock
sudo mkdir build && cd build
sudo cmake .. && sudo make install
cd .. && sudo rm -rf build
```

## CentOS/Fedora系列

- 同样是先安装clang11及其以上版本。安装方法见：[Fedora Snapshot Packages](https://copr.fedorainfracloud.org/coprs/g/fedora-llvm-team/llvm-snapshots/)
- 安装cmake、libaio(可选)。

```bash
# Optional.
sudo yum install cmake libaio-devel -y
```

- 编译安装gtest、gmock。

```
git clone git@github.com:google/googletest.git -b v1.8.x
cd googletest
mkdir build && cd build
cmake .. && sudo make install
```

## macOS

- 建议升级macOS 12.1，默认已安装clang13。
- 安装cmake，googletest。

```
brew install cmake
brew install googletest
```

## 源码依赖

- 如果你不是上述Linux发行版，可以直接源码编译来安装依赖组件。

```bash
# libaio (optional)
git clone https://pagure.io/libaio.git
cd libaio
sudo make install
# gmock/gtest
git clone git@github.com:google/googletest.git -b v1.8.x
cd googletest
mkdir build && cd build
cmake .. && sudo make install
```

# 编译运行

- 准备好编译器和依赖组件后就可以开始编译运行async\_simple。
- 首先克隆async\_simple代码并进入代码目录，然后执行如下命令编译。
- 编译成功结束后，运行单元测试以确保能够正常运行。

```bash
mkdir build && cd build
# Specify [-DASYNC_SIMPLE_ENABLE_TESTS=OFF] to skip tests.
CXX=clang++ CC=clang cmake ../ -DCMAKE_BUILD_TYPE=[Release|Debug] [-DASYNC_SIMPLE_ENABLE_TESTS=OFF]
make -j4
make test
make install
```

# 更多示例

接下来可以阅读更多API文档，例如C++20协程（[Lazy](./docs/docs.cn/Lazy.md)）。熟悉async\_simple组件，可以先跟着[Demo](./docs/docs.cn/GetStarted.md)用C++20协程来实现异步统计文件字符数功能。

更多文档介绍见目录[docs](./docs/docs.cn)。

# 存在问题？

如果在使用时遇到任何问题，我们建议先阅读 [文档](./docs/docs.cn), [issues](https://github.com/alibaba/async_simple/issues)
以及 [discussions](https://github.com/alibaba/async_simple/discussions).
如果你没有找到满意的答案，可以提一个 [Issue](https://github.com/alibaba/async_simple/issues) 或者在 [discussions](https://github.com/alibaba/async_simple/discussions) 中开启讨论。具体地，对于缺陷报告以及功能增强，我们建议提一个 [Issue](https://github.com/alibaba/async_simple/issues)；而对于如何使用等问题，我们建议在 [discussions](https://github.com/alibaba/async_simple/discussions) 中开启讨论。

# 示例依赖

注意目前示例代码包含了单独[asio](https://github.com/chriskohlhoff/asio/tree/master/asio)代码。对应Commit: f70f65ae54351c209c3a24704624144bfe8e70a3

# 如何贡献

- 提前阅读下文档：[如何修复Issue](./docs/docs.en/HowToFixIssue.md)。
- 确保修改后单元测试通过，代码格式化通过。（`git-clang-format HEAD^`）
- 创建并提交Pull Request，选择开发者Review代码。（候选人：ChuanqiXu9, RainMark, foreverhy, qicosmos）
- 最终意见一致后，代码将会被合并。

# 性能监控

我们可在 https://alibaba.github.io/async_simple/benchmark-monitoring/index.html 中检查每个 commit 前后的性能。

# 许可证

async\_simple遵循Apache License (Version 2.0)开源许可证。async\_simple包含的部分第三方组件可能遵循其它开源许可证。相关详细信息可以查看NOTICE文件。