#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/detail/error_code.hpp>
#include <memory>

#include "db/i_journal_repository.hpp"

namespace Http {
namespace net = boost::asio;
using tcp = net::ip::tcp;

class HttpServer : public std::enable_shared_from_this<HttpServer> {
 public:
  HttpServer(net::io_context& acceptor_ioc, net::io_context& worker_ioc,
             const tcp::endpoint& endpoint, IJournalRepository& repository);
  void run();
  void stop();

 private:
  void do_accept();
  void on_accept(boost::system::error_code ec, tcp::socket socket);

  tcp::acceptor acceptor_;
  net::io_context& worker_ioc_;
  IJournalRepository& repository_;
};

}  // namespace Http