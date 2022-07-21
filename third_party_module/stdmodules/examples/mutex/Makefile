build: mutex.cpp
	clang++ -std=c++20 -fprebuilt-module-path=../.. mutex.cpp -c -o mutex.o 
	clang++ -std=c++20 mutex.o -o mutex.out  -L../.. -lstd -pthread

clean:
	rm -f *.out *.o *.pcm
