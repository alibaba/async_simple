build: thread.cpp
	clang++ -std=c++20 -fprebuilt-module-path=../.. thread.cpp -c -o thread.o
	clang++ -std=c++20 thread.o -o thread.out  -L../.. -lstd -pthread

clean:
	rm -f *.out *.o *.pcm
