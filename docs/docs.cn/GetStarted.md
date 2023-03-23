# Get Started

本文档通过计算一个文档中特定字符数量的 demo 程序来介绍如何使用 Lazy 进行编程。

创建空文件 `CountChar.cpp`, 并填入以下内容

```cpp
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"

using namespace async_simple::coro;

using Texts = std::vector<std::string>;

Lazy<Texts> ReadFile(const std::string &filename) {
    Texts Result;
    std::ifstream infile(filename);
    std::string line;
    while (std::getline(infile, line))
        Result.push_back(line);
    co_return Result;
}

Lazy<int> CountLineChar(const std::string &line, char c) {
    co_return std::count(line.begin(), line.end(), c);
}

Lazy<int> CountTextChar(const Texts &Content, char c) {
    int Result = 0;
    for (const auto &line : Content)
        Result += co_await CountLineChar(line, c);
    co_return Result;
}

Lazy<int> CountFileCharNum(const std::string &filename, char c) {
    Texts Contents = co_await ReadFile(filename);
    co_return co_await CountTextChar(Contents, c);
}

int main() {
    int Num = syncAwait(CountFileCharNum("file.txt", 'x'));
    std::cout << "The number of 'x' in file.txt is " << Num << "\n";
    return 0;
}

```

同时执行命令：
```
base64 /dev/urandom | head -c 10000 > file.txt
```
这个命令会在 `file.txt` 中随机生成大小为 10000 字节的文本文件。

之后编译并执行:
```
clang++ -std=c++20 CountChar.cpp -o CountChar
./CountChar
```

之后 `CountChar` 会计算出 `file.txt` 中 `x` 的数量。 至此，我们就完成了一个使用协程计算文本字符数量的程序了。

## 使用自带的 CountChar Example

CountChar Example 可在 async_simple/demo_example 中找到。在构建之后，在 build 目录中执行：
```
./demo_example/CountChar
```
即可运行该 example。
