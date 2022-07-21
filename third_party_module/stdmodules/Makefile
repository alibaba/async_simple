INTERFACE_OPTION=--precompile -fprebuilt-module-path=. -g -DNDEBUG -O2

build: std.pcm libstd.a

std.pcm: std.cppm coroutine.cppm numeric.cppm iostream.cppm string.cppm vector.cppm thread.cppm chrono.cppm \
map.cppm mutex.cppm atomic.cppm random.cppm cstdio.cppm fstream.cppm memory.cppm stdexcept.cppm utility.cppm \
array.cppm algorithm.cppm functional.cppm type_traits.cppm cstdint.cppm exception.cppm iterator.cppm list.cppm \
condition_variable.cppm queue.cppm cstdlib.cppm tuple.cppm initializer_list.cppm limits.cppm unordered_map.cppm
	clang++ -std=c++20 $(INTERFACE_OPTION) coroutine.cppm -o std-coroutine.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) numeric.cppm -o std-numeric.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) iostream.cppm -o std-iostream.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) string.cppm -o std-string.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) vector.cppm -o std-vector.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) thread.cppm -o std-thread.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) chrono.cppm -o std-chrono.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) map.cppm -o std-map.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) mutex.cppm -o std-mutex.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) atomic.cppm -o std-atomic.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) random.cppm -o std-random.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) cstdio.cppm -o std-cstdio.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) fstream.cppm -o std-fstream.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) memory.cppm -o std-memory.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) stdexcept.cppm -o std-stdexcept.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) utility.cppm -o std-utility.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) array.cppm -o std-array.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) algorithm.cppm -o std-algorithm.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) functional.cppm -o std-functional.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) type_traits.cppm -o std-type_traits.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) cstdint.cppm -o std-cstdint.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) exception.cppm -o std-exception.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) iterator.cppm -o std-iterator.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) list.cppm -o std-list.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) condition_variable.cppm -o std-condition_variable.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) queue.cppm -o std-queue.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) cstdlib.cppm -o std-cstdlib.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) tuple.cppm -o std-tuple.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) initializer_list.cppm -o std-initializer_list.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) limits.cppm -o std-limits.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) unordered_map.cppm -o std-unordered_map.pcm
	clang++ -std=c++20 $(INTERFACE_OPTION) std.cppm -o std.pcm

libstd.a:std.pcm
	clang++ -std=c++20 std-coroutine.pcm -c -o std-coroutine.o
	clang++ -std=c++20 std-numeric.pcm -c -o std-numeric.o
	clang++ -std=c++20 std-iostream.pcm -c -o std-iostream.o
	clang++ -std=c++20 std-string.pcm -c -o std-string.o
	clang++ -std=c++20 std-vector.pcm -c -o std-vector.o
	clang++ -std=c++20 std-thread.pcm -c -o std-thread.o
	clang++ -std=c++20 std-chrono.pcm -c -o std-chrono.o
	clang++ -std=c++20 std-map.pcm -c -o std-map.o
	clang++ -std=c++20 std-mutex.pcm -c -o std-mutex.o
	clang++ -std=c++20 std-atomic.pcm -c -o std-atomic.o
	clang++ -std=c++20 std-random.pcm -c -o std-random.o
	clang++ -std=c++20 std-cstdio.pcm -c -o std-cstdio.o
	clang++ -std=c++20 std-fstream.pcm -c -o std-fstream.o
	clang++ -std=c++20 std-memory.pcm -c -o std-memory.o
	clang++ -std=c++20 std-stdexcept.pcm -c -o std-stdexcept.o
	clang++ -std=c++20 std-utility.pcm -c -o std-utility.o
	clang++ -std=c++20 std-algorithm.pcm -c -o std-algorithm.o
	clang++ -std=c++20 std-array.pcm -c -o std-array.o
	clang++ -std=c++20 std-functional.pcm -c -o std-functional.o
	clang++ -std=c++20 std-type_traits.pcm -c -o std-type_traits.o
	clang++ -std=c++20 std-cstdint.pcm -c -o std-cstdint.o
	clang++ -std=c++20 std-exception.pcm -c -o std-exception.o
	clang++ -std=c++20 std-iterator.pcm -c -o std-iterator.o
	clang++ -std=c++20 std-list.pcm -c -o std-list.o
	clang++ -std=c++20 std-condition_variable.pcm -c -o std-condition_variable.o
	clang++ -std=c++20 std-queue.pcm -c -o std-queue.o
	clang++ -std=c++20 std-cstdlib.pcm -c -o std-cstdlib.o
	clang++ -std=c++20 std-tuple.pcm -c -o std-tuple.o
	clang++ -std=c++20 std-initializer_list.pcm -c -o std-initializer_list.o
	clang++ -std=c++20 std-limits.pcm -c -o std-limits.o
	clang++ -std=c++20 std-unordered_map.pcm -c -o std-unordered_map.o
	clang++ -std=c++20 std.pcm -c -o std.o
	llvm-ar crv libstd.a std-coroutine.o std-numeric.o std-iostream.o std-string.o std-vector.o \
std-thread.o std-chrono.o std-map.o std-mutex.o std-atomic.o std-random.o std-cstdio.o std-fstream.o \
std-memory.o std-stdexcept.o std-utility.o std-algorithm.o std-array.o std-functional.o std-type_traits.o \
std-cstdint.o std-exception.o std-iterator.o std-list.o std-condition_variable.o std-queue.o std-cstdlib.o \
std-tuple.o std-initializer_list.o std-limits.o std-unordered_map.o std.o 

clean:
	rm -f *.o *.out *.a *.so
	rm -f *.pcm
	cd examples && make clean
