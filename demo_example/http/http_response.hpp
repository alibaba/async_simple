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
#ifndef ASYNC_SIMPLE_HTTP_RESPONSE_HPP
#define ASYNC_SIMPLE_HTTP_RESPONSE_HPP

#include <string_view>
#include <unordered_map>

/// The status of the response.
enum class status_type {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
};

namespace status_line {

constexpr std::string_view status_line_ok = "HTTP/1.1 200 OK\r\n";
constexpr std::string_view status_line_created = "HTTP/1.1 201 Created\r\n";
constexpr std::string_view status_line_accepted = "HTTP/1.1 202 Accepted\r\n";
constexpr std::string_view status_line_no_content =
    "HTTP/1.1 204 No Content\r\n";
constexpr std::string_view status_line_multiple_choices =
    "HTTP/1.1 300 Multiple Choices\r\n";
constexpr std::string_view status_line_moved_permanently =
    "HTTP/1.1 301 Moved Permanently\r\n";
constexpr std::string_view status_line_moved_temporarily =
    "HTTP/1.1 302 Moved Temporarily\r\n";
constexpr std::string_view status_line_not_modified =
    "HTTP/1.1 304 Not Modified\r\n";
constexpr std::string_view status_line_bad_request =
    "HTTP/1.1 400 Bad Request\r\n";
constexpr std::string_view status_line_unauthorized =
    "HTTP/1.1 401 Unauthorized\r\n";
constexpr std::string_view status_line_forbidden = "HTTP/1.1 403 Forbidden\r\n";
constexpr std::string_view status_line_not_found = "HTTP/1.1 404 Not Found\r\n";
constexpr std::string_view status_line_internal_server_error =
    "HTTP/1.1 500 Internal Server Error\r\n";
constexpr std::string_view status_line_not_implemented =
    "HTTP/1.1 501 Not Implemented\r\n";
constexpr std::string_view status_line_bad_gateway =
    "HTTP/1.1 502 Bad Gateway\r\n";
constexpr std::string_view status_line_service_unavailable =
    "HTTP/1.1 503 Service Unavailable\r\n";

asio::const_buffer status_to_buffer(status_type status) {
    switch (status) {
        case status_type::ok:
            return asio::buffer(status_line_ok);
        case status_type::created:
            return asio::buffer(status_line_created);
        case status_type::accepted:
            return asio::buffer(status_line_accepted);
        case status_type::no_content:
            return asio::buffer(status_line_no_content);
        case status_type::multiple_choices:
            return asio::buffer(status_line_multiple_choices);
        case status_type::moved_permanently:
            return asio::buffer(status_line_moved_permanently);
        case status_type::moved_temporarily:
            return asio::buffer(status_line_moved_temporarily);
        case status_type::not_modified:
            return asio::buffer(status_line_not_modified);
        case status_type::bad_request:
            return asio::buffer(status_line_bad_request);
        case status_type::unauthorized:
            return asio::buffer(status_line_unauthorized);
        case status_type::forbidden:
            return asio::buffer(status_line_forbidden);
        case status_type::not_found:
            return asio::buffer(status_line_not_found);
        case status_type::internal_server_error:
            return asio::buffer(status_line_internal_server_error);
        case status_type::not_implemented:
            return asio::buffer(status_line_not_implemented);
        case status_type::bad_gateway:
            return asio::buffer(status_line_bad_gateway);
        case status_type::service_unavailable:
            return asio::buffer(status_line_service_unavailable);
        default:
            return asio::buffer(status_line_internal_server_error);
    }
}

}  // namespace status_line

namespace mime_types {
const std::unordered_map<std::string_view, std::string_view> mime_map = {
    {"gif", "image/gif"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {"jpg", "image/jpeg"},
    {"png", "image/png"}};

std::string_view extension_to_type(std::string_view extension) {
    if (auto it = mime_map.find(extension); it != mime_map.end()) {
        return it->second;
    }
    return "text/plain";
}

}  // namespace mime_types

namespace misc_strings {
constexpr std::string_view name_value_separator = ": ";
constexpr std::string_view crlf = "\r\n";
}  // namespace misc_strings

struct response {
    /// The status of the response.
    status_type status;

    /// The headers to be included in the reply.
    std::vector<header> headers;

    /// The content to be sent in the reply.
    std::string content;

    std::vector<asio::const_buffer> to_buffers() {
        std::vector<asio::const_buffer> buffers;
        buffers.push_back(status_line::status_to_buffer(status));
        for (std::size_t i = 0; i < headers.size(); ++i) {
            header& h = headers[i];
            buffers.push_back(asio::buffer(h.name));
            buffers.push_back(asio::buffer(misc_strings::name_value_separator));
            buffers.push_back(asio::buffer(h.value));
            buffers.push_back(asio::buffer(misc_strings::crlf));
        }
        buffers.push_back(asio::buffer(misc_strings::crlf));
        buffers.push_back(asio::buffer(content));
        return buffers;
    }
};

namespace response_content {
constexpr std::string_view response_ok =
    "<html>"
    "<head><title>Hello</title></head>"
    "<body><h1>Hello async_simple</h1></body>"
    "</html>";
constexpr std::string_view response_created =
    "<html>"
    "<head><title>Created</title></head>"
    "<body><h1>201 Created</h1></body>"
    "</html>";
constexpr std::string_view response_accepted =
    "<html>"
    "<head><title>Accepted</title></head>"
    "<body><h1>202 Accepted</h1></body>"
    "</html>";
constexpr std::string_view response_no_content =
    "<html>"
    "<head><title>No Content</title></head>"
    "<body><h1>204 Content</h1></body>"
    "</html>";
constexpr std::string_view response_multiple_choices =
    "<html>"
    "<head><title>Multiple Choices</title></head>"
    "<body><h1>300 Multiple Choices</h1></body>"
    "</html>";
constexpr std::string_view response_moved_permanently =
    "<html>"
    "<head><title>Moved Permanently</title></head>"
    "<body><h1>301 Moved Permanently</h1></body>"
    "</html>";
constexpr std::string_view response_moved_temporarily =
    "<html>"
    "<head><title>Moved Temporarily</title></head>"
    "<body><h1>302 Moved Temporarily</h1></body>"
    "</html>";
constexpr std::string_view response_not_modified =
    "<html>"
    "<head><title>Not Modified</title></head>"
    "<body><h1>304 Not Modified</h1></body>"
    "</html>";
constexpr std::string_view response_bad_request =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
constexpr std::string_view response_unauthorized =
    "<html>"
    "<head><title>Unauthorized</title></head>"
    "<body><h1>401 Unauthorized</h1></body>"
    "</html>";
constexpr std::string_view response_forbidden =
    "<html>"
    "<head><title>Forbidden</title></head>"
    "<body><h1>403 Forbidden</h1></body>"
    "</html>";
constexpr std::string_view response_not_found =
    "<html>"
    "<head><title>Not Found</title></head>"
    "<body><h1>404 Not Found</h1></body>"
    "</html>";
constexpr std::string_view response_internal_server_error =
    "<html>"
    "<head><title>Internal Server Error</title></head>"
    "<body><h1>500 Internal Server Error</h1></body>"
    "</html>";
constexpr std::string_view response_not_implemented =
    "<html>"
    "<head><title>Not Implemented</title></head>"
    "<body><h1>501 Not Implemented</h1></body>"
    "</html>";
constexpr std::string_view response_bad_gateway =
    "<html>"
    "<head><title>Bad Gateway</title></head>"
    "<body><h1>502 Bad Gateway</h1></body>"
    "</html>";
constexpr std::string_view response_service_unavailable =
    "<html>"
    "<head><title>Service Unavailable</title></head>"
    "<body><h1>503 Service Unavailable</h1></body>"
    "</html>";

std::string_view to_string(status_type status) {
    switch (status) {
        case status_type::ok:
            return response_ok;
        case status_type::created:
            return response_created;
        case status_type::accepted:
            return response_accepted;
        case status_type::no_content:
            return response_no_content;
        case status_type::multiple_choices:
            return response_multiple_choices;
        case status_type::moved_permanently:
            return response_moved_permanently;
        case status_type::moved_temporarily:
            return response_moved_temporarily;
        case status_type::not_modified:
            return response_not_modified;
        case status_type::bad_request:
            return response_bad_request;
        case status_type::unauthorized:
            return response_unauthorized;
        case status_type::forbidden:
            return response_forbidden;
        case status_type::not_found:
            return response_not_found;
        case status_type::internal_server_error:
            return response_internal_server_error;
        case status_type::not_implemented:
            return response_not_implemented;
        case status_type::bad_gateway:
            return response_bad_gateway;
        case status_type::service_unavailable:
            return response_service_unavailable;
        default:
            return response_internal_server_error;
    }
}
}  // namespace response_content

response build_response(status_type status) {
    response rep;
    rep.status = status;
    rep.content = response_content::to_string(status);
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = std::to_string(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = "text/html";
    return rep;
}

#endif