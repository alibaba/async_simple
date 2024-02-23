.PHONY:clean build 

build: third_party_module/stdmodules/std.pcm third_party_module/asio/asio.pcm  async_simple_module/async_simple.pcm demo_example_module/block_http_server.out

third_party_module/stdmodules/std.pcm:
	cd third_party_module/stdmodules && $(MAKE)

third_party_module/asio/asio.pcm:
	cd third_party_module/asio && $(MAKE)

async_simple_module/async_simple.pcm: third_party_module/stdmodules/std.pcm
	cd async_simple_module && $(MAKE)

demo_example_module/block_http_server.out: third_party_module/asio/asio.pcm async_simple_module/async_simple.pcm
	cd demo_example_module && $(MAKE)

clean:
	find . -name "*.o" -o -name "*.out" -o -name "*.pcm" | xargs rm -f
	cd third_party_module && $(MAKE) clean
	cd async_simple_module && $(MAKE) clean
	cd demo_example_module && $(MAKE) clean
