
#include "http/http_session.hpp"

#include <sys/socket.h>

#include <boost/beast/core/error.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/core/ignore_unused.hpp>
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include "http/json_serializer.hpp"
#include "http/url_utils.hpp"

namespace Http {

HttpSession::HttpSession(tcp::socket&& socket, IJournalRepository& repository)
    : stream_(std::move(socket)), repository_(repository) {}

void HttpSession::run() { do_read(); }

void HttpSession::do_read() {
  req_ = {};
  stream_.expires_after(std::chrono::seconds(30));

  http::async_read(stream_, buffer_, req_,
                   [self = shared_from_this()](beast::error_code ec,
                                               size_t bytes_transferred) {
                     self->on_read(ec, bytes_transferred);
                   });
}

void HttpSession::on_read(beast::error_code ec, size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec == http::error::end_of_stream) {
    do_close();
    return;
  }
  if (ec) {
    return;
  }
  handle_request();
}

void HttpSession::handle_request() {
  if (req_.method() != http::verb::get) {
    send_response(http::status::bad_request,
                  serialize_error_to_json("Only GET method is supported"));
    return;
  }

  auto [path, query_str] = split_target(req_.target());
  if (path != "/journal") {
    send_response(http::status::not_found,
                  serialize_error_to_json("Resource not found"));
    return;
  }
  auto params = parse_query_string(query_str);

  std::string timestamp;
  if (auto it = params.find("since");
      it != params.end() && !it->second.empty()) {
    timestamp = it->second;
  } else {
    send_response(http::status::bad_request,
                  serialize_error_to_json("Missing reaured paramter: 'since'"));
    return;
  }

  int level = 0;
  if (auto it = params.find("level");
      it != params.end() && !it->second.empty()) {
    try {
      level = std::stoi(it->second);
      if (level < 0) {
        send_response(http::status::bad_request,
                      serialize_error_to_json("Parameter 'level' must be >=0"));
        return;
      }
    } catch (const std::exception&) {
      send_response(http::status::bad_request,
                    serialize_error_to_json("Invalid parameter 'level'"));
      return;
    }
  }

  try {
    constexpr size_t kLimit = 10;
    auto records = repository_.get_records(level, timestamp, kLimit);
    std::string json_body = serialize_records_to_json(records);
    send_response(http::status::ok, json_body);
  } catch (const std::exception& e) {
    send_response(http::status::internal_server_error,
                  serialize_error_to_json("Internal server error"));
    std::cerr << e.what() << '\n';
  }
}

void HttpSession::send_response(http::status status, std::string body) {
  res_ = std::make_shared<http::response<http::string_body>>(status,
                                                             req_.version());
  res_->set(http::field::server, "JournalServer");
  res_->set(http::field::content_type, "application/json");
  res_->keep_alive(req_.keep_alive());
  res_->body() = std::move(body);
  res_->prepare_payload();

  http::async_write(stream_, *res_,
                    [self = shared_from_this()](beast::error_code ec,
                                                size_t bytes_transferred) {
                      self->on_write(self->res_->keep_alive(), ec,
                                     bytes_transferred);
                    });
}

void HttpSession::on_write(bool keep_alive, beast::error_code ec,
                           size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);
  if (ec) {
    return;
  }
  if (!keep_alive) {
    do_close();
    return;
  }
  do_read();
}

void HttpSession::do_close() {
  beast::error_code ec;
  boost::ignore_unused(
      stream_.socket().shutdown(tcp::socket::shutdown_send, ec));
}

}  // namespace Http