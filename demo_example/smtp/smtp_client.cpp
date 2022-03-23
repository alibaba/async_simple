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

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "../asio_util.hpp"

#ifdef ENABLE_SSL
#include "asio/ssl.hpp"
#endif

// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static inline bool is_base64(char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(const std::string &str) {
    std::string ret;
    int i = 0;
    int j = 0;
    char char_array_3[3];
    char char_array_4[4];

    auto bytes_to_encode = str.data();
    size_t in_len = str.size();
    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) +
                              ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) +
                              ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}
std::string base64_decode(std::string const &encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') &&
           is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] =
                (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) +
                              ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] =
            (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] =
            ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            ret += char_array_3[j];
    }

    return ret;
}

enum class Security { NonSSL, SSL };

namespace smtp {

struct email_server {
    std::string server;
    std::string port;
    std::string user;
    std::string password;
};
struct email_data {
    std::string from_email;
    std::vector<std::string> to_email;
    std::string subject;
    std::string text;
    std::string filepath;
};

template <Security security>
class client {
public:
    static constexpr bool IS_SSL = (security == Security::SSL);
    client(asio::io_context &io_context)
        : io_context_(io_context), socket_(io_context), resolver_(io_context) {}

    ~client() { close(); }

    void set_email_server(const email_server &server) { server_ = server; }
    void set_email_data(const email_data &data) { data_ = data; }

    async_simple::coro::Lazy<void> send_email() {
        std::string host = server_.server;
        size_t pos = host.find("://");
        if (pos != std::string::npos) {
            host.erase(0, pos + 3);
        }

        asio::ip::tcp::resolver::query qry(
            host, server_.port, asio::ip::resolver_query_base::numeric_service);
        std::error_code ec;
        auto endpoint_iterator = resolver_.resolve(qry, ec);

        ec = co_await async_connect(io_context_, socket_, host, server_.port);
        if (ec) {
            std::cout << "Connect error: " << ec.message() << '\n';
            throw asio::system_error(ec);
        }

        std::cout << "connect ok\n";
        if constexpr (IS_SSL) {
            upgrade_to_ssl();
        }

        build_request();

        const char *header = asio::buffer_cast<const char *>(request_.data());
        std::cout << header << '\n';
        [[maybe_unused]] auto [write_ec, size] = co_await async_write(
            get_socket(), asio::buffer(header, request_.size()));
        if (write_ec) {
            std::cout << "write failed, reason: " << ec.message() << '\n';
            throw asio::system_error(write_ec);
        }

        while (true) {
            const int max_length = 1024;
            char read_buf[max_length];
            [[maybe_unused]] auto [read_ec, size] = co_await async_read_some(
                get_socket(), asio::buffer(read_buf, max_length));
            if (read_ec) {
                std::cout << "read failed, reason: " << ec.message() << '\n';
                throw asio::system_error(read_ec);
            }

            std::string_view content(read_buf, size);
            std::cout << content;

            if (content.find("250 Mail OK") != std::string_view::npos) {
                std::cout << "end smtp\n";
                break;
            }
        }
    }

private:
    auto &get_socket() {
#ifdef ENABLE_SSL
        if constexpr (IS_SSL) {
            assert(ssl_socket_);
            return *ssl_socket_;
        } else
#endif
        {
            return socket_;
        }
    }

    void upgrade_to_ssl() {
#ifdef ENABLE_SSL
        asio::ssl::context ctx(asio::ssl::context::sslv23);
        ctx.set_default_verify_paths();
        ctx.set_verify_mode(ctx.verify_fail_if_no_peer_cert);

        ssl_socket_ =
            std::make_unique<asio::ssl::stream<asio::ip::tcp::socket &>>(
                socket_, ctx);
        ssl_socket_->set_verify_mode(asio::ssl::verify_none);
        ssl_socket_->set_verify_callback([](auto preverified, auto &ctx) {
            char subject_name[256];
            X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
            X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
            std::cout << "subject_name: " << subject_name << '\n';
            return preverified;
        });

        asio::error_code ec;
        ssl_socket_->handshake(asio::ssl::stream_base::client, ec);
#endif
    }

    std::string load_file_contents(const std::string &filepath) {
        std::ifstream fin(filepath.c_str(), std::ios::in | std::ios::binary);
        if (!fin) {
            throw std::invalid_argument(filepath + " not exist");
        }

        std::ostringstream oss;
        oss << fin.rdbuf();
        return oss.str();
    }

    void build_smtp_content(std::ostream &out) {
        out << "Content-Type: multipart/mixed; "
               "boundary=\"async_simple\"\r\n\r\n";
        out << "--async_simple\r\nContent-Type: text/plain;\r\n\r\n";
        out << data_.text << "\r\n\r\n";
    }

    void build_smtp_file(std::ostream &out) {
        if (data_.filepath.empty()) {
            return;
        }

        std::string filename = data_.filepath.substr(
            data_.filepath.find_last_of(get_separator()) + 1);
        out << "--async_simple\r\nContent-Type: application/octet-stream; "
               "name=\""
            << filename << "\"\r\n";
        out << "Content-Transfer-Encoding: base64\r\n";
        out << "Content-Disposition: attachment; filename=\"" << filename
            << "\"\r\n";
        out << "\r\n";

        std::string file_content = load_file_contents(data_.filepath);
        size_t file_size = file_content.size();

        std::string encoded = base64_encode(file_content);

        int SEND_BUF_SIZE = 1024;

        int no_of_rows = (int)file_size / SEND_BUF_SIZE + 1;

        for (int i = 0; i != no_of_rows; ++i) {
            std::string sub_buf =
                encoded.substr(i * SEND_BUF_SIZE, SEND_BUF_SIZE);

            out << sub_buf << "\r\n";
        }
    }

    void build_request() {
        std::ostream out(&request_);

        out << "EHLO " << server_.server << "\r\n";
        out << "AUTH LOGIN\r\n";
        out << base64_encode(server_.user) << "\r\n";
        out << base64_encode(server_.password) << "\r\n";
        out << "MAIL FROM:<" << data_.from_email << ">\r\n";
        for (auto to : data_.to_email)
            out << "RCPT TO:<" << to << ">\r\n";
        out << "DATA\r\n";
        out << "FROM: " << data_.from_email << "\r\n";
        for (auto to : data_.to_email)
            out << "TO: " << to << "\r\n";
        out << "SUBJECT: " << data_.subject << "\r\n";

        build_smtp_content(out);
        build_smtp_file(out);

        out << "--async_simple--\r\n";
        out << ".\r\n";
    }

    void close() {
        asio::error_code ignore_ec;
        if constexpr (IS_SSL) {
#ifdef ENABLE_SSL
            ssl_socket_->shutdown(ignore_ec);
#endif
        }

        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
        socket_.close(ignore_ec);
    }

private:
    std::string get_separator() {
#ifdef _WIN32
        return '\\';
#endif
        return "/";
    }
    asio::io_context &io_context_;
    asio::ip::tcp::socket socket_;
#ifdef ENABLE_SSL
    std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket &>> ssl_socket_;
#endif
    asio::ip::tcp::resolver resolver_;

    email_server server_;
    email_data data_;

    asio::streambuf request_;
    asio::streambuf response_;
};

}  // namespace smtp

template <Security security>
auto get_client(asio::io_context &io_context) {
    return smtp::client<security>(io_context);
}

int main(int argc, char **argv) {
    smtp::email_server server{};
    server.server = "smtp.163.com";
#ifdef ENABLE_SSL
    server.port = "465";
#else
    server.port = "25";
#endif

    server.user = "your_email@163.com";
    server.password = "your_email_password";

    smtp::email_data data{};
    data.filepath = "test_file.txt";
    data.from_email = "your_email@163.com";
    data.to_email.push_back("to_some_email@163.com");
    data.to_email.push_back("to_more_email@example.com");

    data.subject = "Test async_simple smtp sending email";
    data.text = "Hi";

    try {
        asio::io_context io_context;
        std::thread thd([&io_context] {
            asio::io_context::work work(io_context);
            io_context.run();
        });

#ifdef ENABLE_SSL
        auto client = get_client<Security::SSL>(io_context);
#else
        auto client = get_client<Security::NonSSL>(io_context);
#endif
        client.set_email_server(server);
        client.set_email_data(data);
        async_simple::coro::syncAwait(client.send_email());
        io_context.stop();
        thd.join();
        std::cout << "Finished ok, client quit.\n";
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
