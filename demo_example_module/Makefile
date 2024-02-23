.PHONY:clean build ReadFiles CountChar async-httpd httpd http

STD_MODULE_PATH=../third_party_module/stdmodules
OPT_FLAGS ?= -O2 -DNDEBUG -g $(STL_PATH) -fprebuilt-module-path=$(STD_MODULE_PATH) -Wno-unused-command-line-argument 
OBJECT_CFLAGS ?= -std=c++20 -Wall $(OPT_FLAGS)
INTERFACE_CFLAGS ?= $(OBJECT_CFLAGS) --precompile $(STL_PATH)
LINK_CFLAGS =
EXECUTABLE_CFLAGS ?= $(OBJECT_CFLAGS) $(LINK_CFLAGS)

TEST_DEPENDENCY_LIB = -lpthread -laio -lasync_simple -lstd
TEST_DEPENDENCY_DIR = -L../async_simple_module -L/usr/local/lib64 -L$(STD_MODULE_PATH)
DEPENDENCY = $(TEST_DEPENDENCY_DIR) $(TEST_DEPENDENCY_LIB) -Wl,-rpath=../async_simple_module

ASIO_PATH ?= ../third_party_module/asio

PREBUILT_PATH = -fprebuilt-module-path=../async_simple_module \
				-fprebuilt-module-path=../async_simple_module/coro \
				-fprebuilt-module-path=../async_simple_module/executors \
				-fprebuilt-module-path=../async_simple_module/util \
				-fprebuilt-module-path=../async_simple_module/uthread \
				-fprebuilt-module-path=../async_simple_module/uthread/internal \
				-fprebuilt-module-path=$(ASIO_PATH) \
				-fprebuilt-module-path=./

build: ReadFiles.out CountChar.out async_echo_client.out async_echo_server.out block_echo_client.out block_echo_server.out http_client.out http_server.out block_http_server.out

ReadFiles:ReadFiles.out
	./ReadFiles.out

CountChar:CountChar.out
	./CountChar.out

ReadFiles.out: ReadFiles.cpp ../async_simple_module/async_simple.o
	clang++ $(EXECUTABLE_CFLAGS) ReadFiles.cpp $(DEPENDENCY) $(PREBUILT_PATH) -o ReadFiles.out

CountChar.out: CountChar.cpp ../async_simple_module/async_simple.o
	clang++ $(EXECUTABLE_CFLAGS) CountChar.cpp $(DEPENDENCY) $(PREBUILT_PATH) -o CountChar.out

async_echo_client.o: async_echo_client.cpp asio_util.pcm
	clang++ $(OBJECT_CFLAGS) async_echo_client.cpp $(PREBUILT_PATH) -c -o async_echo_client.o

async_echo_client.out: async_echo_client.o asio_util.o
	clang++ $(EXECUTABLE_CFLAGS) async_echo_client.o asio_util.o $(ASIO_PATH)/asio.o $(DEPENDENCY) $(PREBUILT_PATH) -o async_echo_client.out

async_echo_server.o: async_echo_server.cpp asio_util.pcm
	clang++ $(OBJECT_CFLAGS) async_echo_server.cpp $(PREBUILT_PATH)  -c -o async_echo_server.o

async_echo_server.out: async_echo_server.o asio_util.o
	clang++ $(EXECUTABLE_CFLAGS) async_echo_server.o asio_util.o $(ASIO_PATH)/asio.o $(DEPENDENCY) $(PREBUILT_PATH) -o async_echo_server.out

block_echo_client.o: async_echo_client.cpp asio_util.pcm
	clang++ $(OBJECT_CFLAGS) async_echo_client.cpp $(PREBUILT_PATH) -c -o block_echo_client.o

block_echo_client.out: block_echo_client.o asio_util.o
	clang++ $(EXECUTABLE_CFLAGS) block_echo_client.o asio_util.o $(ASIO_PATH)/asio.o $(DEPENDENCY) -o block_echo_client.out

block_echo_server.o: async_echo_server.cpp asio_util.pcm
	clang++ $(OBJECT_CFLAGS) async_echo_server.cpp $(PREBUILT_PATH) -c -o block_echo_server.o

block_echo_server.out: block_echo_server.o asio_util.o
	clang++ $(EXECUTABLE_CFLAGS) block_echo_server.o asio_util.o $(ASIO_PATH)/asio.o $(DEPENDENCY) -o block_echo_server.out

http_client.out: http_client.o asio_util.o
	clang++ $(EXECUTABLE_CFLAGS) http_client.o asio_util.o $(ASIO_PATH)/asio.o $(DEPENDENCY) -o http_client.out 

http_client.o: ./http/coroutine_http/http_client.cpp asio_util.pcm
	clang++ $(OBJECT_CFLAGS) ./http/coroutine_http/http_client.cpp $(PREBUILT_PATH) -c -o http_client.o 

http_server.out: http_server.o asio_util.o
	clang++ $(EXECUTABLE_CFLAGS) http_server.o asio_util.o $(ASIO_PATH)/asio.o $(DEPENDENCY) -o http_server.out

http_server.o: ./http/coroutine_http/http_server.cpp asio_util.pcm
	clang++ $(OBJECT_CFLAGS) ./http/coroutine_http/http_server.cpp $(PREBUILT_PATH) -c -o http_server.o

block_http_server.out: block_http_server.o asio_util.o
	clang++ $(EXECUTABLE_CFLAGS) block_http_server.o asio_util.o $(ASIO_PATH)/asio.o $(DEPENDENCY) -o block_http_server.out

block_http_server.o: ./http/block_http/block_http_server.cpp asio_util.pcm
	clang++ $(OBJECT_CFLAGS) ./http/block_http/block_http_server.cpp $(PREBUILT_PATH) -c -o block_http_server.o

asio_util.o: asio_util.pcm
	clang++ $(OBJECT_CFLAGS) asio_util.cppm $(PREBUILT_PATH) -c -o asio_util.o

asio_util.pcm: asio_util.cppm ../async_simple_module/async_simple.pcm $(ASIO_PATH)/asio.pcm
	clang++ $(INTERFACE_CFLAGS) asio_util.cppm $(PREBUILT_PATH) -c -o asio_util.pcm

$(ASIO_PATH)/asio.pcm: 
	+ cd $(ASIO_PATH) && $(MAKE)

clean:
	find . -name "*.o" -o -name "*.out" -o -name "*.pcm" | xargs rm -f
