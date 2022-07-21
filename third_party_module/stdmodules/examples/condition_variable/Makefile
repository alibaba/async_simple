build: condition_variable.cpp
	clang++ -std=c++20 -fprebuilt-module-path=../.. condition_variable.cpp -c -o condition_variable.o
	clang++ -std=c++20 condition_variable.o -o condition_variable.out  -L../.. -lstd -pthread

clean:
	rm -f *.out *.o *.pcm
