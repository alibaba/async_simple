# Get Started

The docs shows a demo to use Lazy to calculate the number of specified char in a program.

Create a file with name `CountChar.cpp` and fill the following codes.

```C++
#include "async_simple/coro/Lazy.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

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

Execute:
```
base64 /dev/urandom | head -c 10000 > file.txt
```
This would generate a text file called with 10000 bytes randomly.

The compile it as:
```
clang++ -std=c++20 CountChar.cpp -o CountChar
./CountChar
```

Then `CountChar` would calculate the number of `x` in `file.txt`.

## Run CountChar Example

We could find CountChar example in async_simple/demo_example. After building async_simple, we could run the following command in build directory to run the demo:
```
./demo_example/CountChar
```

