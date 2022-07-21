build: scoped_lock.cpp
	clang++ -std=c++20 -fprebuilt-module-path=../.. scoped_lock.cpp -c -o scoped_lock.o
	clang++ -std=c++20 scoped_lock.o -o scoped_lock.out  -L../.. -lstd -pthread

clean:
	rm -f *.out *.o *.pcm
