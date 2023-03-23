# Method

Due to the reason that there is no production level executor in async_simple, we choose to compare the creating speed and switching speed to test the performance.

## Things to compare

We use `Lazy` in async_simple, `Task` in folly and `task` in cppcoro to compare in this documents.

## Hardware

```
CPU: Intel(R) Xeon(R) Platinum 8163 CPU @ 2.50GHz
cpu MHz		: 2699.584
processor number: 96
CPU Caches:
  L1 Data 32 KiB (x48)
  L1 Instruction 32 KiB (x48)
  L2 Unified 1024 KiB (x48)
  L3 Unified 33792 KiB (x2)
```

## Compiler

Clang13

# Test Program

## Call Chain

Simulates the speed of coroutine chain.
```cpp
template<template<typename> typename LazyType, int N>
struct lazy_fn {
    LazyType<int> operator()() {
        co_return N + co_await lazy_fn<LazyType, N-1>()();
    }
};

template<template<typename> typename LazyType>
struct lazy_fn<LazyType, 0> {
    LazyType<int> operator()() {
        co_return 1;
    }
};

Lazy<int> foo() {
    co_return co_await lazy_fn<Lazy, 1000>()();;
};
```

## CollectAll

Simulates the speed of concurrent coroutines.
```cpp
Lazy<void> foo() {
    std::vector<Lazy<int>> lazies;
    for (int i = 0; i < 5000; i++)
        lazies.push_back(lazy_fn<Lazy, 50>()());
    co_await collectAllPara(std::move(lazies));
};
```

## Complete Program

Here is the testing program. It uses google/benchmark.

```cpp
template<template<typename> typename LazyType, int N>
struct lazy_fn {
    LazyType<int> operator()() {
        co_return N + co_await lazy_fn<LazyType, N-1>()();
    }
};

template<template<typename> typename LazyType>
struct lazy_fn<LazyType, 0> {
    LazyType<int> operator()() {
        co_return 1;
    }
};

void async_simple_Lazy_chain(benchmark::State& state) {
  auto chain_starter = [&]() -> async_simple::coro::Lazy<int> {
    co_return co_await lazy_fn<async_simple::coro::Lazy, 1000>()();;
  };
  for (const auto& _ : state)
    async_simple::coro::syncAwait(chain_starter());
}


void async_simple_Lazy_collectAll(benchmark::State& state) {
  auto collectAllStarter = [&]() -> async_simple::coro::Lazy<void> {
    std::vector<async_simple::coro::Lazy<int>> lazies;
    for (int i = 0; i < 5000; i++)
      lazies.push_back(lazy_fn<async_simple::coro::Lazy, 50>()());
    co_await async_simple::coro::collectAllPara(std::move(lazies));
  };
  for (const auto& _ : state)
    syncAwait(collectAllStarter());
}

void FollyTaskChain(benchmark::State& state) {
  auto chain_starter = [&]() -> folly::coro::Task<int> {
    co_return co_await lazy_fn<folly::coro::Task, 1000>()();;
  };
  for (const auto& _ : state)
    folly::coro::blockingWait(chain_starter());
}

void FollyTaskCollectAll(benchmark::State& state) {
  auto collectAllStarter = [&]() -> folly::coro::Task<void> {
    std::vector<folly::coro::Task<int>> tasks;
    for (int i = 0; i < 5000; i++)
      tasks.push_back(lazy_fn<folly::coro::Task, 50>()());
    co_await folly::coro::collectAllRange(std::move(tasks));
  };
  for (const auto& _ : state)
    folly::coro::blockingWait(collectAllStarter());
}

void cppcoro_task_chain(benchmark::State& state) {
  auto chain_starter = [&]() -> cppcoro::task<int> {
    co_return co_await lazy_fn<cppcoro::task, 1000>()();;
  };
  for (const auto& _ : state)
    cppcoro::sync_wait(chain_starter());
}

void cppcoro_task_when_all(benchmark::State& state) {
  auto collectAllStarter = [&]() -> cppcoro::task<void> {
    std::vector<cppcoro::task<int>> tasks;
    for (int i = 0; i < 5000; i++)
      tasks.push_back(lazy_fn<cppcoro::task, 50>()());
    co_await cppcoro::when_all(std::move(tasks));
  };
  for (const auto& _ : state)
    cppcoro::sync_wait(collectAllStarter());
}

void async_simple_Lazy_chain(benchmark::State& state);
void FollyTaskChain(benchmark::State& state);
void cppcoro_task_chain(benchmark::State& state);
void async_simple_Lazy_collectAll(benchmark::State& state);
void FollyTaskCollectAll(benchmark::State& state);
void cppcoro_task_when_all(benchmark::State& state);

BENCHMARK(FollyTaskChain);
BENCHMARK(cppcoro_task_chain);
BENCHMARK(async_simple_Lazy_chain);
BENCHMARK(FollyTaskCollectAll);
BENCHMARK(cppcoro_task_when_all);
BENCHMARK(async_simple_Lazy_collectAll);
BENCHMARK_MAIN();
```

# Result

```
----------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations
----------------------------------------------------------------------
FollyTaskChain                  195801 ns       193211 ns         3616
cppcoro_task_chain               61308 ns        60614 ns        11542
async_simple_Lazy_chain           59745 ns        59086 ns        11846
FollyTaskCollectAll           23795927 ns     23555262 ns           30
cppcoro_task_when_all          8934768 ns      8829864 ns           79
async_simple_Lazy_collectAll    7880137 ns      7785291 ns           90
```

We could found that the performance of Lazy is not bad.
Note that this is only a very simple test for showing the async_simple. It didn't mean to say async_simple is better.
And an important thing to note that is `Task` in Folly has more functionality. For example, `Task` would record the context when switching to enhance the debugbility.
