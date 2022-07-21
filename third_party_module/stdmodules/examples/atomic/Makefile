build: atomic.cpp
	clang++ -std=c++20 -fprebuilt-module-path=../.. atomic.cpp -c -o atomic.o 
	clang++ -std=c++20 atomic.o -o atomic.out  -L../.. -lstd -pthread

clean:
	rm -f *.out *.o *.pcm
