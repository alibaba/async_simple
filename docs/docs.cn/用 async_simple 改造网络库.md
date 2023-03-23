用async_simple可以轻松改造同步网络库从而获得大幅性能提升，用它改造异步回调网络库可以让我们以同步方式写代码，让代码更简洁，可读性更好，还能避免callback hell的问题。

本文将通过两个例子分别来介绍如何用async_simple改造基于asio的同步网络库和异步回调网络库。

demo example依赖了独立版的[asio](https://github.com/chriskohlhoff/asio/tree/master/asio)(commit id:f70f65ae54351c209c3a24704624144bfe8e70a3), 它是header only的直接包含头文件即可。

# 改造同步网络库
以一个同步的echo server/client为例，先来看看echo server：
```cpp
void start_server(asio::io_context& io_context, unsigned short port) {
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
    for (;;) {
        auto [error, socket] = accept(a);
        session(std::move(socket));
    }
}
```
echo server启动时先监听端口，然后同步accept，当有新连接到来时在session中处理网络读写事件。
```cpp
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
在一个循环中先同步读数据，再将读到的数据发送到对端。

同步的echo client
```cpp
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
同步的echo client也是类似的过程，先连接，再循环发送数据和读数据。整个逻辑都是同步的，代码简单易懂。

当然，因为整个过程都是同步等待的，所以无法并发的处理网IO事件，性能是比较差的。如果用async_simple来改造这个同步的网络库则可以大幅提升网络库的性能(参考[benchmark](#benchmark))，而且**需要修改的代码很少**，这就是async_simple的威力。

## 用async_simple改造同步echo server

```cpp
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
可以看到用async_simple改造的start_server和session相比，返回类型变成了Lazy, 同步接口变成了co_await async_xxx，原有的同步逻辑没有任何变化，这个微小的改动即可让我们把同步网络库改成异步协程网络库从而让性能得到巨大的提升。

更多的代码细节请看demo_example中echo server/client的例子。

# 改造异步回调网络库
对于一些已有的异步回调网络库，也可以用async_simple来消除回调，从而让我们可以用同步方式去使用异步接口，让代码变得更简洁易懂。

以asio异步[http server](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp11/http/server/connection.cpp)为例：
```cpp
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
可以看到基于回调的异步网络库的代码比同步网络库复杂很多，如在异步读的回调中如果没有读到完整数据需要递归调用do_read，如果读到完整数据之后才能在回调中调用异步写接口。同时，还要注意将shared_from_this()得到的std::shared_ptr传入到异步接口的回调函数中以保证安全的回调。总之，异步回调代码的编写难度较大，可读性也较差，如果用async_simple改造则可以较好的解决这些问题，性能也不会降低。

## 用async_simple改造异步http server
```cpp
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
用async_simple改造之后的http server完全消除了回调函数，完全是以同步方式去写异步代码，简洁易懂。

# 基本测试
启动http_server，在浏览器中输入: 127.0.0.1:9980/，将在页面看到“hello async_simple”的字符串；

如果希望返回index.html则需要将demo_example/http/index.html文件拷贝到http_server目录，输入: 127.0.0.1:9980/index.html，将返回index.html文件的内容。如果找不到index.html将会返回"not found"。

# Benchmark
测试方法：向http_server(127.0.0.1:9980/)发送http请求，将收到"hello async_simple"的响应字符串，用wrk对它进行压测。

## 测试工具
下载编译[wrk](https://github.com/wg/wrk)
```
git clone https://github.com/wg/wrk.git
cd wrk
make
```

## 测试命令行
```
./wrk -t10 -c1000 -d30s http://127.0.0.1:9980/
```
机器配置：8c16g Intel(R) Xeon(R) CPU E5-2682 v4 @ 2.50GHz

## 测试block_http_server
启动block_http_server: ./block_http_server

用wrk测试：./wrk -t10 -c1000 -d30s http://127.0.0.1:9980/

## 测试block_http_server的结果
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

## 测试async_simple改造之后的http_server
启动http_server: ./http_server

用wrk测试：./wrk -t10 -c1000 -d30s http://127.0.0.1:9980/

## 测试http_server的结果
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

## 比较测试结果
| 同步http_server qps | 协程异步http_server qps | 性能提升 |
| ------ | ------ | ------ |
| **32483** | **72363** | **2.2倍** |
