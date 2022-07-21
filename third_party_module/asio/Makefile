OPT_FLAGS ?= -O2 -DNDEBUG -g
OBJECT_CFLAGS ?= -std=c++20 -Wall -Werror $(OPT_FLAGS)
INTERFACE_CFLAGS ?= $(OBJECT_CFLAGS) --precompile

build: asio.cppm
	clang++ -std=c++20 $(INTERFACE_CFLAGS) -c -I./ asio.cppm -o asio.pcm
	clang++ -std=c++20 $(OBJECT_CFLAGS) -c asio.pcm -c -o asio.o
clean:
	rm -f *.o *.out
	rm -f *.pcm
	rm -f core.*
	rm -fr gcm.cache
