.PHONY:clean build 

build: third_party_module/stdmodules/std.pcm third_party_module/asio/asio.pcm  async_simple_module/async_simple.pcm demo_example_module/block_http_server.out

third_party_module/stdmodules/std.pcm:
	cd third_party_module/stdmodules && make

third_party_module/asio/asio.pcm:
	cd third_party_module/asio && make

async_simple_module/async_simple.pcm: third_party_module/stdmodules/std.pcm
	cd async_simple_module && make

demo_example_module/block_http_server.out: third_party_module/asio/asio.pcm async_simple_module/async_simple.pcm
	cd demo_example_module && make

clean:
	find . -name "*.o" -o -name "*.out" -o -name "*.pcm" | xargs rm -f
	cd third_party_module && make clean
	cd async_simple_module && make clean
	cd demo_example_module && make clean
