build: unique_lock.cpp
	clang++ -std=c++20 -fprebuilt-module-path=../.. unique_lock.cpp -c -o unique_lock.o
	clang++ -std=c++20 unique_lock.o -o unique_lock.out  -L../.. -lstd -pthread

clean:
	rm -f *.out *.o *.pcm
