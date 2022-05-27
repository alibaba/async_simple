.PHONY:clean test build build_all build_test

export OPT_FLAGS = -O2 -g -DNDEBUG -fmodules_aggressive_function_reduction  -flto=thin
export OBJECT_CFLAGS = -std=c++20 $(OPT_FLAGS) -Werror
export SHARED_OBJECT_CFLAGS = $(OBJECT_CFLAGS) -fPIC
export INTERFACE_CFLAGS = $(OBJECT_CFLAGS) --precompile 

test: build_all
	+ cd ./test && make

build_all: build build_test
	
build: async_simple.pcm libasync_simple.so libasync_simple.a

build_test: build
	+ cd ./test && make build

clean:
	find . -name "*.o" -o -name "*.out" -o -name "*.pcm" | xargs rm -f
	+ cd ./test && make clean

libasync_simple.so: ./uthread/internal/thread-PIC.o ./uthread/internal/jump_x86_64_sysv_elf_gas-PIC.o ./uthread/internal/make_x86_64_sysv_elf_gas-PIC.o async_simple.o ./util/async_simple-util.ThreadPool-PIC.o ./executors/async_simple-executors.SimpleExecutor-PIC.o async_simple-Executor-PIC.o ./executors/async_simple-executors.SimpleIOExecutor-PIC.o ./util/async_simple-util.Condition-PIC.o ./coro/async_simple-coro.Util-PIC.o ./coro/async_simple-coro.Lazy-PIC.o ./coro/async_simple-coro.CoAwait-PIC.o ./uthread/async_simple-uthread.Latch-PIC.o ./uthread/async_simple-uthread.Uthread-PIC.o ./coro/async_simple-coro.Collect-PIC.o ./coro/async_simple-coro.Event-PIC.o experimental/experimental.coroutine-PIC.o experimental/experimental.coroutine-PIC.o async_simple-Try-PIC.o async_simple-IOExecutor-PIC.o async_simple-Common-PIC.o ./uthread/internal/async_simple-uthread.thread_impl-PIC.o async_simple-Future-PIC.o async_simple-FutureState-PIC.o ./uthread/internal/async_simple-uthread.thread-PIC.o ./util/async_simple-util.Queue-PIC.o
	clang++ $(OPT_FLAGS) -fuse-ld=lld -shared -o libasync_simple.so ./uthread/internal/thread-PIC.o ./uthread/internal/jump_x86_64_sysv_elf_gas-PIC.o ./uthread/internal/make_x86_64_sysv_elf_gas-PIC.o async_simple.o ./util/async_simple-util.ThreadPool-PIC.o ./executors/async_simple-executors.SimpleExecutor-PIC.o async_simple-Executor-PIC.o ./executors/async_simple-executors.SimpleIOExecutor-PIC.o ./util/async_simple-util.Condition-PIC.o ./coro/async_simple-coro.Util-PIC.o ./coro/async_simple-coro.Lazy-PIC.o ./coro/async_simple-coro.CoAwait-PIC.o ./uthread/async_simple-uthread.Latch-PIC.o ./uthread/async_simple-uthread.Uthread-PIC.o ./coro/async_simple-coro.Collect-PIC.o ./coro/async_simple-coro.Event-PIC.o experimental/experimental.coroutine-PIC.o async_simple-Try-PIC.o async_simple-IOExecutor-PIC.o async_simple-Common-PIC.o ./uthread/internal/async_simple-uthread.thread_impl-PIC.o async_simple-Future-PIC.o async_simple-FutureState-PIC.o ./uthread/internal/async_simple-uthread.thread-PIC.o ./util/async_simple-util.Queue-PIC.o

libasync_simple.a: ./uthread/internal/thread.o ./uthread/internal/jump_x86_64_sysv_elf_gas.o ./uthread/internal/make_x86_64_sysv_elf_gas.o async_simple.o ./util/async_simple-util.ThreadPool.o ./executors/async_simple-executors.SimpleExecutor.o async_simple-Executor.o ./executors/async_simple-executors.SimpleIOExecutor.o ./util/async_simple-util.Condition.o ./coro/async_simple-coro.Util.o ./coro/async_simple-coro.Lazy.o ./coro/async_simple-coro.CoAwait.o ./uthread/async_simple-uthread.Latch.o ./uthread/async_simple-uthread.Uthread.o ./coro/async_simple-coro.Collect.o ./coro/async_simple-coro.Event.o experimental/experimental.coroutine.o experimental/experimental.coroutine.o async_simple-Try.o async_simple-IOExecutor.o async_simple-Common.o ./uthread/internal/async_simple-uthread.thread_impl.o async_simple-Future.o async_simple-FutureState.o ./uthread/internal/async_simple-uthread.thread.o ./util/async_simple-util.Queue.o
	llvm-ar crv libasync_simple.a ./uthread/internal/thread.o ./uthread/internal/jump_x86_64_sysv_elf_gas.o ./uthread/internal/make_x86_64_sysv_elf_gas.o async_simple.o ./util/async_simple-util.ThreadPool.o ./executors/async_simple-executors.SimpleExecutor.o async_simple-Executor.o ./executors/async_simple-executors.SimpleIOExecutor.o ./util/async_simple-util.Condition.o ./coro/async_simple-coro.Util.o ./coro/async_simple-coro.Lazy.o ./coro/async_simple-coro.CoAwait.o ./uthread/async_simple-uthread.Latch.o ./uthread/async_simple-uthread.Uthread.o ./coro/async_simple-coro.Collect.o ./coro/async_simple-coro.Event.o experimental/experimental.coroutine.o async_simple-Try.o async_simple-IOExecutor.o async_simple-Common.o ./uthread/internal/async_simple-uthread.thread_impl.o async_simple-Future.o async_simple-FutureState.o ./uthread/internal/async_simple-uthread.thread.o ./util/async_simple-util.Queue.o

async_simple.o: async_simple.pcm
	clang++ $(OBJECT_CFLAGS)  -c async_simple.pcm -o async_simple.o

./util/async_simple-util.ThreadPool-PIC.o: ./util/async_simple-util.ThreadPool.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./util/async_simple-util.ThreadPool.pcm -o ./util/async_simple-util.ThreadPool-PIC.o

./util/async_simple-util.ThreadPool.o: ./util/async_simple-util.ThreadPool.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./util/async_simple-util.ThreadPool.pcm -o ./util/async_simple-util.ThreadPool.o

./util/async_simple-util.Queue-PIC.o: ./util/async_simple-util.Queue.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./util/async_simple-util.Queue.pcm -o ./util/async_simple-util.Queue-PIC.o

./util/async_simple-util.Queue.o: ./util/async_simple-util.Queue.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./util/async_simple-util.Queue.pcm -o ./util/async_simple-util.Queue.o

./executors/async_simple-executors.SimpleExecutor-PIC.o: ./executors/async_simple-executors.SimpleExecutor.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./executors/async_simple-executors.SimpleExecutor.pcm -o ./executors/async_simple-executors.SimpleExecutor-PIC.o

./executors/async_simple-executors.SimpleExecutor.o: ./executors/async_simple-executors.SimpleExecutor.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./executors/async_simple-executors.SimpleExecutor.pcm -o ./executors/async_simple-executors.SimpleExecutor.o

./executors/async_simple-executors.SimpleIOExecutor-PIC.o: ./executors/async_simple-executors.SimpleIOExecutor.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./executors/async_simple-executors.SimpleIOExecutor.pcm -o ./executors/async_simple-executors.SimpleIOExecutor-PIC.o

./executors/async_simple-executors.SimpleIOExecutor.o: ./executors/async_simple-executors.SimpleIOExecutor.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./executors/async_simple-executors.SimpleIOExecutor.pcm -o ./executors/async_simple-executors.SimpleIOExecutor.o

async_simple-Executor-PIC.o: async_simple-Executor.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c async_simple-Executor.pcm -o async_simple-Executor-PIC.o

async_simple-Executor.o: async_simple-Executor.pcm
	clang++ $(OBJECT_CFLAGS)  -c async_simple-Executor.pcm -o async_simple-Executor.o

async_simple-IOExecutor-PIC.o: async_simple-IOExecutor.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c async_simple-IOExecutor.pcm -o async_simple-IOExecutor-PIC.o

async_simple-IOExecutor.o: async_simple-IOExecutor.pcm
	clang++ $(OBJECT_CFLAGS)  -c async_simple-IOExecutor.pcm -o async_simple-IOExecutor.o

./util/async_simple-util.Condition-PIC.o: ./util/async_simple-util.Condition.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./util/async_simple-util.Condition.pcm -o ./util/async_simple-util.Condition-PIC.o

./util/async_simple-util.Condition.o: ./util/async_simple-util.Condition.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./util/async_simple-util.Condition.pcm -o ./util/async_simple-util.Condition.o

./coro/async_simple-coro.Util-PIC.o: ./coro/async_simple-coro.Util.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./coro/async_simple-coro.Util.pcm -o ./coro/async_simple-coro.Util-PIC.o

./coro/async_simple-coro.Util.o: ./coro/async_simple-coro.Util.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./coro/async_simple-coro.Util.pcm -o ./coro/async_simple-coro.Util.o

./coro/async_simple-coro.Lazy-PIC.o: ./coro/async_simple-coro.Lazy.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./coro/async_simple-coro.Lazy.pcm -o ./coro/async_simple-coro.Lazy-PIC.o

./coro/async_simple-coro.Lazy.o: ./coro/async_simple-coro.Lazy.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./coro/async_simple-coro.Lazy.pcm -o ./coro/async_simple-coro.Lazy.o

./coro/async_simple-coro.Collect-PIC.o: ./coro/async_simple-coro.Collect.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./coro/async_simple-coro.Collect.pcm -o ./coro/async_simple-coro.Collect-PIC.o

./coro/async_simple-coro.Collect.o: ./coro/async_simple-coro.Collect.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./coro/async_simple-coro.Collect.pcm -o ./coro/async_simple-coro.Collect.o

./coro/async_simple-coro.CoAwait-PIC.o: ./coro/async_simple-coro.CoAwait.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./coro/async_simple-coro.CoAwait.pcm -o ./coro/async_simple-coro.CoAwait-PIC.o

./coro/async_simple-coro.CoAwait.o: ./coro/async_simple-coro.CoAwait.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./coro/async_simple-coro.CoAwait.pcm -o ./coro/async_simple-coro.CoAwait.o

./coro/async_simple-coro.Event-PIC.o: ./coro/async_simple-coro.Event.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./coro/async_simple-coro.Event.pcm -o ./coro/async_simple-coro.Event-PIC.o

./coro/async_simple-coro.Event.o: ./coro/async_simple-coro.Event.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./coro/async_simple-coro.Event.pcm -o ./coro/async_simple-coro.Event.o

./uthread/async_simple-uthread.Latch-PIC.o: ./uthread/async_simple-uthread.Latch.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./uthread/async_simple-uthread.Latch.pcm -o ./uthread/async_simple-uthread.Latch-PIC.o

./uthread/async_simple-uthread.Latch.o: ./uthread/async_simple-uthread.Latch.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./uthread/async_simple-uthread.Latch.pcm -o ./uthread/async_simple-uthread.Latch.o

./uthread/async_simple-uthread.Uthread-PIC.o: ./uthread/async_simple-uthread.Uthread.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./uthread/async_simple-uthread.Uthread.pcm -o ./uthread/async_simple-uthread.Uthread-PIC.o

./uthread/async_simple-uthread.Uthread.o: ./uthread/async_simple-uthread.Uthread.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./uthread/async_simple-uthread.Uthread.pcm -o ./uthread/async_simple-uthread.Uthread.o

experimental/experimental.coroutine-PIC.o: experimental/experimental.coroutine.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c experimental/experimental.coroutine.pcm -o experimental/experimental.coroutine-PIC.o

experimental/experimental.coroutine.o: experimental/experimental.coroutine.pcm
	clang++ $(OBJECT_CFLAGS)  -c experimental/experimental.coroutine.pcm -o experimental/experimental.coroutine.o

async_simple-Future-PIC.o: async_simple-Future.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c async_simple-Future.pcm -o async_simple-Future-PIC.o

async_simple-Future.o: async_simple-Future.pcm
	clang++ $(OBJECT_CFLAGS)  -c async_simple-Future.pcm -o async_simple-Future.o

async_simple-FutureState-PIC.o: async_simple-FutureState.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c async_simple-FutureState.pcm -o async_simple-FutureState-PIC.o

async_simple-FutureState.o: async_simple-FutureState.pcm
	clang++ $(OBJECT_CFLAGS)  -c async_simple-FutureState.pcm -o async_simple-FutureState.o

async_simple-Try-PIC.o: async_simple-Try.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c async_simple-Try.pcm -o async_simple-Try-PIC.o

async_simple-Try.o: async_simple-Try.pcm
	clang++ $(OBJECT_CFLAGS)  -c async_simple-Try.pcm -o async_simple-Try.o

async_simple-Common-PIC.o: async_simple-Common.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c async_simple-Common.pcm -o async_simple-Common-PIC.o

async_simple-Common.o: async_simple-Common.pcm
	clang++ $(OBJECT_CFLAGS)  -c async_simple-Common.pcm -o async_simple-Common.o

./uthread/internal/async_simple-uthread.thread_impl-PIC.o: ./uthread/internal/async_simple-uthread.thread_impl.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./uthread/internal/async_simple-uthread.thread_impl.pcm -o ./uthread/internal/async_simple-uthread.thread_impl-PIC.o

./uthread/internal/async_simple-uthread.thread_impl.o: ./uthread/internal/async_simple-uthread.thread_impl.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./uthread/internal/async_simple-uthread.thread_impl.pcm -o ./uthread/internal/async_simple-uthread.thread_impl.o

./uthread/internal/async_simple-uthread.thread-PIC.o: ./uthread/internal/async_simple-uthread.thread.pcm
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./uthread/internal/async_simple-uthread.thread.pcm -o ./uthread/internal/async_simple-uthread.thread-PIC.o

./uthread/internal/async_simple-uthread.thread.o: ./uthread/internal/async_simple-uthread.thread.pcm
	clang++ $(OBJECT_CFLAGS)  -c ./uthread/internal/async_simple-uthread.thread.pcm -o ./uthread/internal/async_simple-uthread.thread.o

async_simple.pcm: async_simple.cppm async_simple-Try.pcm async_simple-Future.pcm ./executors/async_simple-executors.SimpleExecutor.pcm\
				 ./coro/async_simple-coro.Lazy.pcm ./coro/async_simple-coro.Collect.pcm ./coro/async_simple-coro.Sleep.pcm ./coro/async_simple-coro.Task.pcm\
				 ./uthread/async_simple-uthread.Collect.pcm ./uthread/async_simple-uthread.Latch.pcm
	clang++ $(INTERFACE_CFLAGS) \
				-fprebuilt-module-path=. -fprebuilt-module-path=./util -fprebuilt-module-path=./coro -fprebuilt-module-path=./executors -fprebuilt-module-path=./uthread \
				-c async_simple.cppm -o async_simple.pcm

# coroutine
experimental/experimental.coroutine.pcm: experimental/coroutine.cppm
	clang++ $(INTERFACE_CFLAGS) -c experimental/coroutine.cppm -o experimental/experimental.coroutine.pcm
# Try
async_simple-Try.pcm: Try.cppm async_simple-Unit.pcm async_simple-Invoke.pcm async_simple-Common.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -c Try.cppm -o async_simple-Try.pcm
async_simple-Common.pcm: Common.cppm
	clang++ $(INTERFACE_CFLAGS) -c Common.cppm -fprebuilt-module-path=. -o async_simple-Common.pcm
async_simple-Invoke.pcm: Invoke.cppm
	clang++ $(INTERFACE_CFLAGS) -c Invoke.cppm -fprebuilt-module-path=. -o async_simple-Invoke.pcm
async_simple-Unit.pcm: Unit.cppm
	clang++ $(INTERFACE_CFLAGS) -c Unit.cppm -o async_simple-Unit.pcm
# Future
async_simple-Future.pcm: Future.cppm async_simple-FutureState.pcm async_simple-LocalState.pcm async_simple-Traits.pcm ./uthread/internal/async_simple-uthread.thread_impl.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./util -fprebuilt-module-path=./uthread/internal -c Future.cppm -o async_simple-Future.pcm
async_simple-Traits.pcm: Traits.cppm async_simple-Try.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -c Traits.cppm -o async_simple-Traits.pcm
async_simple-MoveWrapper.pcm: MoveWrapper.cppm ./util/async_simple-util.Condition.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -c MoveWrapper.cppm -o async_simple-MoveWrapper.pcm
async_simple-FutureState.pcm: FutureState.cppm async_simple-Executor.pcm async_simple-MoveWrapper.pcm async_simple-Try.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./util -c FutureState.cppm -o async_simple-FutureState.pcm
async_simple-LocalState.pcm: LocalState.cppm async_simple-Executor.pcm async_simple-MoveWrapper.pcm async_simple-Try.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./util -c LocalState.cppm -o async_simple-LocalState.pcm
# Executors
async_simple-IOExecutor.pcm: IOExecutor.cppm
	clang++ $(INTERFACE_CFLAGS) -c IOExecutor.cppm -fprebuilt-module-path=. -o async_simple-IOExecutor.pcm
async_simple-Executor.pcm: Executor.cppm ./util/async_simple-util.Condition.pcm async_simple-IOExecutor.pcm experimental/experimental.coroutine.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./util -fprebuilt-module-path=./experimental -c Executor.cppm -o async_simple-Executor.pcm
# SimpleExecutors
./executors/async_simple-executors.SimpleExecutor.pcm: ./executors/SimpleExecutor.cppm ./executors/async_simple-executors.SimpleIOExecutor.pcm ./util/async_simple-util.ThreadPool.pcm async_simple-Executor.pcm async_simple-IOExecutor.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./executors -fprebuilt-module-path=./util -c ./executors/SimpleExecutor.cppm -o ./executors/async_simple-executors.SimpleExecutor.pcm
./executors/async_simple-executors.SimpleIOExecutor.pcm: ./executors/SimpleIOExecutor.cppm async_simple-IOExecutor.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -c ./executors/SimpleIOExecutor.cppm -o ./executors/async_simple-executors.SimpleIOExecutor.pcm
# util
./util/async_simple-util.Condition.pcm: ./util/Condition.cppm async_simple-Common.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -c ./util/Condition.cppm -o ./util/async_simple-util.Condition.pcm
./util/async_simple-util.Queue.pcm: ./util/Queue.cppm
	clang++ $(INTERFACE_CFLAGS) -c ./util/Queue.cppm -o ./util/async_simple-util.Queue.pcm
./util/async_simple-util.ThreadPool.pcm: ./util/ThreadPool.cppm ./util/async_simple-util.Queue.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=./util/ -c ./util/ThreadPool.cppm -o ./util/async_simple-util.ThreadPool.pcm
# uthread
./uthread/internal/thread.o: ./uthread/internal/thread.cc ./uthread/internal/async_simple-uthread.thread.pcm ./uthread/internal/async_simple-uthread.thread_impl.pcm
	clang++ $(OBJECT_CFLAGS) -fprebuilt-module-path=./uthread/internal/ -fprebuilt-module-path=. -c ./uthread/internal/thread.cc -o ./uthread/internal/thread.o
./uthread/internal/jump_x86_64_sysv_elf_gas.o: ./uthread/internal/jump_x86_64_sysv_elf_gas.S
	clang++ $(OBJECT_CFLAGS)  -c ./uthread/internal/jump_x86_64_sysv_elf_gas.S -o ./uthread/internal/jump_x86_64_sysv_elf_gas.o
./uthread/internal/make_x86_64_sysv_elf_gas.o: ./uthread/internal/make_x86_64_sysv_elf_gas.S
	clang++ $(OBJECT_CFLAGS)  -c ./uthread/internal/make_x86_64_sysv_elf_gas.S -o ./uthread/internal/make_x86_64_sysv_elf_gas.o
./uthread/internal/thread-PIC.o: ./uthread/internal/thread.cc ./uthread/internal/async_simple-uthread.thread.pcm ./uthread/internal/async_simple-uthread.thread_impl.pcm
	clang++ $(SHARED_OBJECT_CFLAGS) -fprebuilt-module-path=./uthread/internal/ -fprebuilt-module-path=. -c ./uthread/internal/thread.cc -o ./uthread/internal/thread-PIC.o
./uthread/internal/jump_x86_64_sysv_elf_gas-PIC.o: ./uthread/internal/jump_x86_64_sysv_elf_gas.S
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./uthread/internal/jump_x86_64_sysv_elf_gas.S -o ./uthread/internal/jump_x86_64_sysv_elf_gas-PIC.o
./uthread/internal/make_x86_64_sysv_elf_gas-PIC.o: ./uthread/internal/make_x86_64_sysv_elf_gas.S
	clang++ $(SHARED_OBJECT_CFLAGS)  -c ./uthread/internal/make_x86_64_sysv_elf_gas.S -o ./uthread/internal/make_x86_64_sysv_elf_gas-PIC.o
./uthread/internal/async_simple-uthread.thread_impl.pcm: ./uthread/internal/thread_impl.cppm
	clang++ $(INTERFACE_CFLAGS) -c ./uthread/internal/thread_impl.cppm -o ./uthread/internal/async_simple-uthread.thread_impl.pcm
./uthread/internal/async_simple-uthread.thread.pcm: ./uthread/internal/thread.cppm ./uthread/internal/async_simple-uthread.thread_impl.pcm async_simple-Future.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=./uthread/internal/ -fprebuilt-module-path=.  -c ./uthread/internal/thread.cppm -o ./uthread/internal/async_simple-uthread.thread.pcm
./uthread/async_simple-uthread.Uthread.pcm: ./uthread/Uthread.cppm ./uthread/internal/async_simple-uthread.thread.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=./uthread/internal/ -fprebuilt-module-path=. -c ./uthread/Uthread.cppm -o ./uthread/async_simple-uthread.Uthread.pcm
./uthread/async_simple-uthread.Latch.pcm: ./uthread/Latch.cppm async_simple-Future.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -c ./uthread/Latch.cppm -o ./uthread/async_simple-uthread.Latch.pcm
./uthread/async_simple-uthread.Async.pcm: ./uthread/Async.cppm ./async_simple-Executor.pcm ./uthread/async_simple-uthread.Uthread.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./uthread/  -c ./uthread/Async.cppm -o ./uthread/async_simple-uthread.Async.pcm
./uthread/async_simple-uthread.Await.pcm: ./uthread/Await.cppm ./async_simple-Future.pcm ./coro/async_simple-coro.Lazy.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./coro/  -c ./uthread/Await.cppm -o ./uthread/async_simple-uthread.Await.pcm
./uthread/async_simple-uthread.Collect.pcm: ./uthread/Collect.cppm ./async_simple-Future.pcm ./uthread/async_simple-uthread.Async.pcm ./uthread/async_simple-uthread.Await.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./uthread/  -c ./uthread/Collect.cppm -o ./uthread/async_simple-uthread.Collect.pcm
# coro
./coro/async_simple-coro.Util.pcm: ./coro/Util.cppm ./experimental/experimental.coroutine.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=./experimental  -fprebuilt-module-path=. -c ./coro/Util.cppm -o ./coro/async_simple-coro.Util.pcm
./coro/async_simple-coro.Traits.pcm: ./coro/Traits.cppm
	clang++ $(INTERFACE_CFLAGS) -c ./coro/Traits.cppm  -fprebuilt-module-path=. -o ./coro/async_simple-coro.Traits.pcm
./coro/async_simple-coro.Task.pcm: ./coro/Task.cppm ./coro/async_simple-coro.Util.pcm ./util/async_simple-util.Condition.pcm ./coro/async_simple-coro.Lazy.pcm ./async_simple-Executor.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=./util -fprebuilt-module-path=./coro -fprebuilt-module-path=./experimental -fprebuilt-module-path=./ -c ./coro/Task.cppm -o ./coro/async_simple-coro.Task.pcm
./coro/async_simple-coro.CoAwait.pcm: ./coro/CoAwait.cppm ./coro/async_simple-coro.Traits.pcm ./async_simple-Executor.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./coro -fprebuilt-module-path=./experimental -c ./coro/CoAwait.cppm -o ./coro/async_simple-coro.CoAwait.pcm
./coro/async_simple-coro.Event.pcm: ./coro/Event.cppm ./async_simple-Executor.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./experimental -c ./coro/Event.cppm -o ./coro/async_simple-coro.Event.pcm
./coro/async_simple-coro.Lazy.pcm: ./coro/Lazy.cppm ./coro/async_simple-coro.Util.pcm ./coro/async_simple-coro.CoAwait.pcm async_simple-Try.pcm async_simple-Executor.pcm ./util/async_simple-util.Condition.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./experimental -fprebuilt-module-path=./coro/ -fprebuilt-module-path=./util/ -c ./coro/Lazy.cppm -o ./coro/async_simple-coro.Lazy.pcm
./coro/async_simple-coro.Collect.pcm: ./coro/Collect.cppm ./coro/async_simple-coro.Event.pcm ./coro/async_simple-coro.Lazy.pcm async_simple-Try.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./experimental -fprebuilt-module-path=./coro/ -c ./coro/Collect.cppm -o ./coro/async_simple-coro.Collect.pcm
./coro/async_simple-coro.Sleep.pcm: ./coro/Sleep.cppm ./coro/async_simple-coro.Lazy.pcm async_simple-Executor.pcm
	clang++ $(INTERFACE_CFLAGS) -fprebuilt-module-path=. -fprebuilt-module-path=./coro/ -c ./coro/Sleep.cppm -o ./coro/async_simple-coro.Sleep.pcm