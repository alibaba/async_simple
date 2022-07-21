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
#ifndef ASYNC_SIMPLE_BLOCK_CONECTION_HPP
#define ASYNC_SIMPLE_BLOCK_CONECTION_HPP

#include "../http_request.hpp"
#include "../http_response.hpp"

class block_connection {
public:
    block_connection(asio::ip::tcp::socket socket, std::string &&doc_root)
        : socket_(std::move(socket)), doc_root_(std::move(doc_root)) {}
    ~block_connection() {
        std::error_code ec;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }
    void start() {
        for (;;) {
            auto [error, bytes_transferred] =
                read_some(socket_, asio::buffer(read_buf_));
            if (error) {
                std::cout << "error: " << error.message()
                          << ", size=" << bytes_transferred << '\n';
                break;
            }

            request_parser::result_type result;
            std::tie(result, std::ignore) = parser_.parse(
                request_, read_buf_, read_buf_ + bytes_transferred);
            if (result == request_parser::good) {
                handle_request(request_, response_);
                write(socket_, response_.to_buffers());
                bool keep_alive = is_keep_alive();
                if (!keep_alive) {
                    break;
                }

                request_ = {};
                response_ = {};
                parser_.reset();
            } else if (result == request_parser::bad) {
                response_ = build_response(status_type::bad_request);
                write(socket_, response_.to_buffers());
                break;
            }
        }
    }

private:
    void handle_request(const request &req, response &rep) {
        // Decode url to path.
        std::string request_path;
        if (!url_decode(req.uri, request_path)) {
            rep = build_response(status_type::bad_request);
            return;
        }

        // Request path must be absolute and not contain "..".
        if (request_path.empty() || request_path[0] != '/' ||
            request_path.find("..") != std::string::npos) {
            rep = build_response(status_type::bad_request);
            return;
        }

        // If path ends in slash (i.e. is a directory) then add "index.html".
        if (request_path[request_path.size() - 1] == '/') {
            rep = build_response(status_type::ok);
            return;
        }

        // Determine the file extension.
        std::size_t last_slash_pos = request_path.find_last_of("/");
        std::size_t last_dot_pos = request_path.find_last_of(".");
        std::string extension;
        if (last_dot_pos != std::string::npos &&
            last_dot_pos > last_slash_pos) {
            extension = request_path.substr(last_dot_pos + 1);
        }

        // Open the file to send back.
        std::string full_path = doc_root_ + request_path;
        std::ifstream is(full_path.c_str(),  std::ios_base::openmode(std::ios::in | std::ios::binary));
        if (!is) {
            rep = build_response(status_type::not_found);
            return;
        }

        // Fill out the response to be sent to the client.
        rep.status = status_type::ok;
        char buf[512];
        while (is.read(buf, sizeof(buf)).gcount() > 0)
            rep.content.append(buf, is.gcount());
        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = std::to_string(rep.content.size());
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = mime_types::extension_to_type(extension);
    }

    bool url_decode(const std::string &in, std::string &out) {
        out.clear();
        out.reserve(in.size());
        for (std::size_t i = 0; i < in.size(); ++i) {
            if (in[i] == '%') {
                if (i + 3 <= in.size()) {
                    int value = 0;
                    std::istringstream is(in.substr(i + 1, 2));
                    if (is >> std::hex >> value) {
                        out += static_cast<char>(value);
                        i += 2;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            } else if (in[i] == '+') {
                out += ' ';
            } else {
                out += in[i];
            }
        }
        return true;
    }

    bool is_keep_alive() {
        for (auto &[k, v] : request_.headers) {
            if (k == "Connection" && v == "close") {
                return false;
                break;
            }
        }
        return true;
    }

private:
    asio::ip::tcp::socket socket_;
    char read_buf_[1024];
    request_parser parser_;
    request request_;
    response response_;
    std::string doc_root_;
};

#endif
