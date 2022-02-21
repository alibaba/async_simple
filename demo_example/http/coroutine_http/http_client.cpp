/*
 * Copyright (c) 2022, Alibaba Group Holding Limited;
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "../../asio_util.hpp"
#include <iostream>
#include <sstream>
#include <string>

using asio::ip::tcp;

async_simple::coro::Lazy<void> start(asio::io_context &io_context,
                                     std::string host, std::string port) {
  asio::ip::tcp::socket socket(io_context);
  auto ec = co_await async_connect(io_context, socket, host, port);
  if (ec) {
    std::cout << "Connect error: " << ec.message() << '\n';
    throw asio::system_error(ec);
  }
  std::cout << "Connect to " << host << ":" << port << " successfully.\n";

  std::stringstream request_stream;
  request_stream << "GET "
                 << "/"
                 << " HTTP/1.1\r\n";
  request_stream << "Host: "
                 << "127.0.0.1"
                 << "\r\n";
  request_stream << "Accept: */*\r\n";
  request_stream << "Connection: close\r\n\r\n";

  // Send the request.
  co_await async_write(
      socket, asio::buffer(request_stream.str(), request_stream.str().size()));

  // Read the response status line. The response streambuf will automatically
  // grow to accommodate the entire line. The growth may be limited by passing
  // a maximum size to the streambuf constructor.
  asio::streambuf response;
  co_await async_read_until(socket, response, "\r\n");

  // Check that response is OK.
  std::istream response_stream(&response);
  std::string http_version;
  response_stream >> http_version;
  unsigned int status_code;
  response_stream >> status_code;
  std::string status_message;
  std::getline(response_stream, status_message);
  if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
    std::cout << "Invalid response\n";
    co_return;
  }
  if (status_code != 200) {
    std::cout << "Response returned with status code " << status_code << "\n";
    co_return;
  }

  // Read the response headers, which are terminated by a blank line.
  co_await async_read_until(socket, response, "\r\n\r\n");

  // Process the response headers.
  std::string header;
  while (std::getline(response_stream, header) && header != "\r")
    std::cout << header << "\n";
  std::cout << "\n";

  // Write whatever content we already have to output.
  if (response.size() > 0)
    std::cout << &response;

  // Read until EOF, writing data to output as we go.
  while (true) {
    auto [error, bytes_transferred] = co_await async_read(socket, response);
    if (error != asio::error::eof)
      throw asio::system_error(error);

    if (bytes_transferred <= 0) {
      break;
    }

    std::cout << &response;
  }

  std::error_code ignore_ec;
  socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
}

// Copy the the file async_simple/demo_example/http/index.html to http server
// directory before run the http client, otherwise you will get 404 not found
// error message.
int main(int argc, char *argv[]) {
  try {
    asio::io_context io_context;
    std::thread thd([&io_context] {
      asio::io_context::work work(io_context);
      io_context.run();
    });
    async_simple::coro::syncAwait(start(io_context, "127.0.0.1", "9980"));
    io_context.stop();
    thd.join();
    std::cout << "Finished ok, client quit.\n";
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}
