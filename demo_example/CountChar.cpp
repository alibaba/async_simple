/*
 * Copyright (c) 2022, Alibaba Group Holding Limited;
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <async_simple/coro/Lazy.h>
#include <async_simple/coro/SyncAwait.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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
    int Num = syncAwait(CountFileCharNum("demo_example/Input/file.txt", 'x'));
    std::cout << "The number of 'x' in file.txt is " << Num << "\n";
    return 0;
}
