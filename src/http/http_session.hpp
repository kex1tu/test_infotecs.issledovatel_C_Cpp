#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <memory>
#include <string>

#include "db/i_journal_repository.hpp"

namespace Http {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = net::ip::tcp;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
 public:
  HttpSession(tcp::socket&& socket, IJournalRepository& repository);
  void run();

 private:
  void do_read();
  void on_read(beast::error_code ec, size_t bytes_transferred);
  void handle_request();
  void send_response(http::status status, std::string body);
  void on_write(bool keep_alive, beast::error_code ec,
                size_t bytes_transferred);
  void do_close();

  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  std::shared_ptr<http::response<http::string_body>> res_;
  IJournalRepository& repository_;
};
}  // namespace Http