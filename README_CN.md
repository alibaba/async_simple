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

# 快速体验

我们可以在 [compiler-explorer](compiler-explorer.com) 中在线快速体验 async_simple: https://compiler-explorer.com/z/Tdaesqsqj . 注意 `Uthread` 在 compiler-explorer 中不能使用。

# 准备环境

async\_simple 涉及 C++20 协程，对编译器版本有较高要求。需要 clang10 或 gcc10 或 Apple-clang14 及其以上版本。编译运行测试需要 gtest，使用`bazel`可自动拉取gtest。需要 libaio 的功能的需要依赖 libaio。

注意当使用 gcc12 时需要加上 `-Wno-maybe-uninitialized` 选项因为 gcc12 的一个误报。详情可见 https://github.com/alibaba/async_simple/issues/234。

注意当使用 clang15 时可能需要加上 `-Wno-unsequenced` 选项，这是clang15的一个误报。详情可见 https://github.com/llvm/llvm-project/issues/56768。

注意，如果使用msvc时遇到了C4737错误，请加上选项/EHa。

## Debian/Ubuntu系列

- 安装clang11及其以上版本。安装方法见：[APT Packages](https://apt.llvm.org/)
- 安装cmake或bazel
- 如需使用 libaio 功能，需安装 libaio。

```bash
sudo apt install libaio-dev -y
# Install cmake
sudo apt install cmake -y
# Install bazel See: https://bazel.build/install/ubuntu
```
- 使用`apt`安装gtest、gmock
```bash
sudo apt install -y libgtest-dev libgmock-dev
```
- [源码编译安装gtest、gmock](#源码依赖)


## CentOS/Fedora系列

- 同样是先安装clang11及其以上版本。安装方法见：[Fedora Snapshot Packages](https://copr.fedorainfracloud.org/coprs/g/fedora-llvm-team/llvm-snapshots/)
- 安装cmake或bazel、libaio(可选)。

```bash
# Optional.
sudo yum install cmake libaio-devel -y
# Install bazel See: https://bazel.build/install/redhat
```
- 使用`yum`安装gtest、gmock
```bash
sudo yum install gtest-devel gmock-devel
```
- [源码编译安装gtest、gmock](#源码依赖)

```
git clone git@github.com:google/googletest.git -b v1.8.x
cd googletest
mkdir build && cd build
cmake .. && sudo make install
```

## Arch系列
```bash
# Optional
sudo pacman -S libaio
# Use cmake
sudo pacman -S cmake gtest
# Use bazel
sudo pacman -S bazel
```

## macOS

- 建议升级macOS 12.1，默认已安装clang13。
- 安装cmake或者bazel，googletest。

```bash
# Use cmake
brew install cmake
brew install googletest
# Use bazel
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

## cmake
```bash
mkdir build && cd build
# Specify [-DASYNC_SIMPLE_ENABLE_TESTS=OFF] to skip tests.
# Specify [-DASYNC_SIMPLE_BUILD_DEMO_EXAMPLE=OFF] to skip build demo example.
# Specify [-DASYNC_SIMPLE_DISABLE_AIO=ON] to skip the build libaio
CXX=clang++ CC=clang cmake ../ -DCMAKE_BUILD_TYPE=[Release|Debug] [-DASYNC_SIMPLE_ENABLE_TESTS=OFF] [-DASYNC_SIMPLE_BUILD_DEMO_EXAMPLE=OFF] [-DASYNC_SIMPLE_DISABLE_AIO=ON]
make -j4
make test
make install
```

我们也支持了conan，你可以将async\_simple安装到conan到本地缓存中。
```
mkdir build && cd build
conan create ..
```

## bazel
```bash
# Specify [--define=ASYNC_SIMPLE_DISABLE_AIO=true] to skip the build libaio
# Example bazel build --define=ASYNC_SIMPLE_DISABLE_AIO=true ...
bazel build ...                     # compile all target
bazel build ...:all                 # compile all target
bazel build ...:*                   # compile all target
bazel build -- ... -benchmarks/...  # compile all target except those beneath `benchmarks`
bazel test ...                      # compile and execute tests
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
- 看[这里](https://bazel.build/run/build)获得`bazel build`的更多信息
- `...`表示递归扫描所有目标，在`oh-my-zsh`中被识别为`../..`，可更换其他`shell`或使用`bash -c 'command.'`运行，例如`bash -c 'bazel build ...'`， 或者使用`bazel build ...:all`
- 使用`async_simple`作为第三方依赖库，也可以看看[bazel support](bazel/support/README.md)

# Docker 编译环境
```
git clone https://github.com/alibaba/async_simple.git
cd async_simple/docker/(ubuntu|centos7|rockylinux)
docker build . --no-cache -t async_simple:1.0
docker run -it --name async_simple async_simple:1.0 /bin/bash
```

# 更多示例

我们的文档托管在GitHub Pages上，[点击进入快速开始](https://alibaba.github.io/async_simple/docs.cn/GetStarted.html).

接下来可以阅读更多API文档，例如C++20协程（[Lazy](./docs/docs.cn/Lazy.md)）。熟悉async\_simple组件，可以先跟着[Demo](./docs/docs.cn/GetStarted.md)用C++20协程来实现异步统计文件字符数功能。

更多文档介绍见目录[docs](./docs/docs.cn)。

# 性能相关

关于基于C++20无栈协程（Lazy）和有栈协程（Uthread）的性能定量分析，详见[定量分析报告](./docs/docs.cn/基于async_simple的协程性能定量分析.md)。

# C++20 Modules

我们在 `modules/async_simple.cppm` 中对 C++20 Modules 做了实验性质的支持。`async_simple` Module 可被 `xmake` 和 `cmake` 编译。我们可以在 `CountChar`, `ReadFiles`, `LazyTest.cpp` 以及 `FutureTest.cpp` 中找到相关的使用方法。

我们需要 clang16 (或 commit 号大于 d18806e6733) 来构建 `async_simple` Module。该功能只在 `libstdc++10.3` 中测试过。鉴于目前 Modules 的支持状况，如果在其他版本的标准库中无法正常使用 `async_simple` Module 并不意外。

我们可以用 xmake (>= 0eccc6e) 来构建 `async_simple` Module：

```
xmake
```

我们可以用 cmake (>= d18806e673 或 cmake3.26)来构建 `async_simple` Module：

```
mkdir build_modules && cd build_modules
CC=clang CXX=clang++ cmake .. -DCMAKE_BUILD_TYPE=Release -DASYNC_SIMPLE_BUILD_MODULES=ON -GNinja
ninja
```

**需要注意:** 出于兼容性考虑，目前 main 分支中的 `async_simple` Module 本质上只是将 `async_simple` 的头文件封装为了 `Named Modules` 而已。我们可以在  https://github.com/alibaba/async_simple/tree/CXX20Modules 中找到更完整的 `Named Modules` 使用方式。该分支中同样包含 xmake 和 cmake 的支持。

# 存在问题？

如果在使用时遇到任何问题，我们建议先阅读 [文档](./docs/docs.cn), [issues](https://github.com/alibaba/async_simple/issues)
以及 [discussions](https://github.com/alibaba/async_simple/discussions).
如果你没有找到满意的答案，可以提一个 [Issue](https://github.com/alibaba/async_simple/issues) 或者在 [discussions](https://github.com/alibaba/async_simple/discussions) 中开启讨论。具体地，对于缺陷报告以及功能增强，我们建议提一个 [Issue](https://github.com/alibaba/async_simple/issues)；而对于如何使用等问题，我们建议在 [discussions](https://github.com/alibaba/async_simple/discussions) 中开启讨论。

# 示例依赖

注意目前示例代码包含了单独[asio](https://github.com/chriskohlhoff/asio/tree/master/asio)代码。对应Commit: f70f65ae54351c209c3a24704624144bfe8e70a3

# 如何贡献

- 提前阅读下文档：[如何修复Issue](./docs/docs.en/HowToFixIssue.md)。
- 确保修改后单元测试通过，代码格式化通过。（`git-clang-format HEAD^`）。目前 CI 系统中的 clang-format 版本为 14。当 CI 中提示的 format 结果不好时，也可考虑暂时先 accept 并在 clang-format 社区提交相关 issue。
- 创建并提交Pull Request，选择开发者Review代码。（候选人：ChuanqiXu9, RainMark, foreverhy, qicosmos）
- 最终意见一致后，代码将会被合并。

# 联系我们

欢迎大家一起共建async_simple, 有问题请扫钉钉二维码加群讨论。  
<center>
<img src="./docs/docs.cn/images/ding_talk_group.png" alt="dingtalk" width="200" height="200" align="bottom" />
</center>

# 许可证

async\_simple遵循Apache License (Version 2.0)开源许可证。async\_simple包含的部分第三方组件可能遵循其它开源许可证。相关详细信息可以查看NOTICE文件。
