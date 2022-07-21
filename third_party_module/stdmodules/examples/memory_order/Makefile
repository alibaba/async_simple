build: memory_order.cpp
	clang++ -std=c++20 -fprebuilt-module-path=../.. memory_order.cpp -c -o memory_order.o 
	clang++ -std=c++20 memory_order.o -o memory_order.out  -L../.. -lstd -pthread

clean:
	rm -f *.out *.o *.pcm
