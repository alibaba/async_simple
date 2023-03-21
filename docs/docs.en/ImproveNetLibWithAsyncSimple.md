You can easily refactor synchronous network libraries to asynchronous libraries with async_simple, and will get great performance improvement, you can also transform asynchronous callback network libraries to coroutine based asynchronous network libraries.

 The coroutine based asynchronous network library allows us write code with synchronous manner, the code will be more shorter, clean, and readable, further more it can help us to avoid the callback hell problem.

This article will show two examples to tell you how to use async_simple to refactor synchronous and asynchronous libraries based on stand-alone asio(commit id:f70f65ae54351c209c3a24704624144bfe8e70a3), which is a header only library.

# Refactor synchronous network libraries
A synchronous echo server：
```c++
void start_server(asio::io_context& io_context, unsigned short port) {
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
    for (;;) {
        auto [error, socket] = accept(a);
        session(std::move(socket));
    }
}
```
The echo server listen a port, and then synchronously wait for accepting, when a new connection comming, handle read/write IO events in session function.
```c++
template <typename AsioBuffer>
std::pair<asio::error_code, size_t> read_some(asio::ip::tcp::socket& sock,
                                              AsioBuffer&& buffer) {
    asio::error_code error;
    size_t length = sock.read_some(std::forward<AsioBuffer>(buffer), error);
    return std::make_pair(error, length);
}

void session(tcp::socket sock) {
    for (;;) {
        const size_t max_length = 1024;
        char data[max_length];
        auto [error, length] = read_some(sock, data, max_length);
        write(sock, data, length);
    }
}
```
Read data and then write it to peer in an infinite loop.

The synchronous echo client
```c++
void start(asio::io_context& io_context, std::string host, std::string port) {
    auto [ec, socket] = connect(io_context, host, port);
    const int max_length = 1024;
    char write_buf[max_length] = {"hello async_simple"};
    char read_buf[max_length];
    const int count = 10000;
    for (int i = 0; i < count; ++i) {
        write(socket, write_buf, max_length);
        auto [error, reply_length] = read_some(socket, read_buf, max_length);
        // handle read data as your wish.
    }
}
```
Similar with the synchronous echo server, the synchronous echo client connect firstly, then wirte and read data in an infinite loop.

The entire logic is synchronous, the code is simple and easy to understand.

Actually, the synchronous echo server can't handle IO events concurrently because of synchronously waiting processing, so the server is inefficient.

We can greatly improve the server's performance with async_simple([see benchmark](#benchmark)), only need to **change very few code**.

## Refactor synchronous echo server with async_simple

```c++
async_simple::coro::Lazy<void> session(tcp::socket sock) {
    for (;;) {
        const size_t max_length = 1024;
        char data[max_length];
        auto [error, length] =
            co_await async_read_some(sock, asio::buffer(data, max_length));
        co_await async_write(sock, asio::buffer(data, length));
    }

    std::error_code ec;
    sock.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    sock.close(ec);

    std::cout << "Finished echo message, total: " << msg_index - 1 << ".\n";
}

async_simple::coro::Lazy<void> start_server(asio::io_context& io_context,
                                           unsigned short port,
                                           async_simple::Executor* E) {
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
    for (;;) {
        auto [error, socket] = co_await async_accept(a);
        session(std::move(socket)).via(E).start([](auto&&) {});
    }
}
```
As you can see, the refactered start_server and session are almost the same with the original functions: call synchronous functions become co_await async_xxx.

The original logic has no change, the very few code modification help us transform a synchronous library to an efficient asynchronous library, that is the power of async_simple.

More details in demo_example echo server/client.

# Refactor asynchronous callback network libraries
For some existing asynchronous callback network library, it also can be refactored to a coroutine-based asynchronous library, the code will be more clean and easier to understand.

Take asio [http server](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp11/http/server/connection.cpp) as an example:
```c++
void connection::do_read()
{
  auto self(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer_),
      [this, self](std::error_code ec, std::size_t bytes_transferred)
      {
        if (!ec)
        {
          request_parser::result_type result;
          std::tie(result, std::ignore) = request_parser_.parse(
              request_, buffer_.data(), buffer_.data() + bytes_transferred);

          if (result == request_parser::good)
          {
            request_handler_.handle_request(request_, reply_);
            do_write();
          }
          else if (result == request_parser::bad)
          {
            reply_ = reply::stock_reply(reply::bad_request);
            do_write();
          }
          else
          {
            do_read();
          }
        }
        else if (ec != asio::error::operation_aborted)
        {
          connection_manager_.stop(shared_from_this());
        }
      });
}

void connection::do_write()
{
  auto self(shared_from_this());
  asio::async_write(socket_, reply_.to_buffers(),
      [this, self](std::error_code ec, std::size_t)
      {
        if (!ec)
        {
          asio::error_code ignored_ec;
          socket_.shutdown(asio::ip::tcp::socket::shutdown_both,
            ignored_ec);
        }

        if (ec != asio::error::operation_aborted)
        {
          connection_manager_.stop(shared_from_this());
        }
      });
}
```
The asynchronous callback is much more complicated than synchronous usage, we have to pay attention to many details: recursive callback, pass shared_from_this() to callback to guarantee asynchronous safe callback. The code is hard to write and understand, we can solve the problems with async_simple, further more, there is no performance degradation.

## Refactor asynchronous http server with async_simple
```c++
    async_simple::coro::Lazy<void> do_read() {
        auto self(shared_from_this());
        for (;;) {
            auto [error, bytes_transferred] =
                co_await async_read_some(socket_, asio::buffer(read_buf_));
            if (error) {
                break;
            }

            request_parser::result_type result;
            std::tie(result, std::ignore) = parser_.parse(
                request_, read_buf_, read_buf_ + bytes_transferred);
            if (result == request_parser::good) {
                handle_request(request_, response_);
                co_await async_write(socket_, response_.to_buffers());
            } else if (result == request_parser::bad) {
                response_ = build_response(status_type::bad_request);
                co_await async_write(socket_, response_.to_buffers());
                break;
            }
        }
    }
```
After refactoring, we reduce all callback functions, the logic is synchronous, the code is short and easy to understand.

# Basic Test
Start http_server, input an address in a brower: 127.0.0.1:9980/, you will look at "hello async_simple" in your browser.

If you want to get the index.html page from http_server, you need to copy index.html into the http_server directory firstly, then input "127.0.0.1:9980/index.html", you will get the cotent of index.html; If the index.html not exist, you will get "not found".

# Benchmark
How to test：Send http request to http_server(127.0.0.1:9980/), the http_server will response "hello async_simple", use wrk to do pressure test.

## Test tool
download and compile [wrk](https://github.com/wg/wrk)
```
git clone https://github.com/wg/wrk.git
cd wrk
make
```

## Test command line
```
./wrk -t10 -c1000 -d30s http://127.0.0.1:9980/
```
Machine: 8c16g Intel(R) Xeon(R) CPU E5-2682 v4 @ 2.50GHz

## Test block_http_server
Start block_http_server: ./block_http_server

Testing with wrk：./wrk -t10 -c1000 -d30s http://127.0.0.1:9980/

## Result of testing block_http_server
```
Running 30s test @ http://127.0.0.1:9980/
  10 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    29.01us   17.46us   8.21ms   96.38%
    Req/Sec    32.72k     2.44k   37.10k    72.67%
  976577 requests in 30.06s, 138.77MB read
Requests/sec:  32483.80
Transfer/sec:      4.62MB
```

## Test http_server1 refactored with async_simple
Start http_server: ./http_server

Testing with wrk：./wrk -t10 -c1000 -d30s http://127.0.0.1:9980/

## Result of testing http_server
```
Running 30s test @ http://127.0.0.1:9980/
  10 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    17.75ms   57.57ms   1.68s    99.17%
    Req/Sec     7.33k     1.14k   15.07k    81.71%
  2172704 requests in 30.03s, 308.74MB read
Requests/sec:  72363.03
Transfer/sec:     10.28MB
```

## Compare benchmark results
| block_http_server qps | coroutine-based http_server1 qps | improvment |
| ------ | ------ | ------ |
| **32483** | **72363** | **2.2x faster** |
