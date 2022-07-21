.PHONY:clean build 

build: stdmodules/std.pcm asio/asio.pcm

stdmodules/std.pcm:
	cd stdmodules && make

asio/asio.pcm:
	cd asio && make

clean:
	find . -name "*.o" -o -name "*.out" -o -name "*.pcm" | xargs rm -f
	cd stdmodules && make clean
	cd asio && make clean
