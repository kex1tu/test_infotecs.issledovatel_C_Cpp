
#include "http/http_server.hpp"

#include <boost/asio/error.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <iostream>
#include <memory>

#include "http/http_session.hpp"

namespace Http {
HttpServer::HttpServer(net::io_context& acceptor_ioc,
                       net::io_context& worker_ioc,
                       const tcp::endpoint& endpoint,
                       IJournalRepository& repository)
    : acceptor_(acceptor_ioc),
      worker_ioc_(worker_ioc),
      repository_(repository) {
  boost::system::error_code ec;
  boost::ignore_unused(acceptor_.open(endpoint.protocol(), ec));
  if (ec) {
    throw boost::system::system_error(ec, "Failed to open acceptor");
  }

  boost::ignore_unused(
      acceptor_.set_option(net::socket_base::reuse_address(true), ec));
  if (ec) {
    throw boost::system::system_error(ec, "Failed to set reuse_address option");
  }

  boost::ignore_unused(acceptor_.bind(endpoint, ec));
  if (ec) {
    throw boost::system::system_error(ec,
                                      "Failed to bind acceptor to endpoint");
  }

  boost::ignore_unused(
      acceptor_.listen(net::socket_base::max_listen_connections, ec));
  if (ec) {
    throw boost::system::system_error(ec, "Failed to listen on acceptor");
  }
}
void HttpServer::run() { do_accept(); }

void HttpServer::stop() {
  boost::system::error_code ec;
  boost::ignore_unused(acceptor_.close(ec));
}

void HttpServer::do_accept() {
  acceptor_.async_accept(net::make_strand(worker_ioc_),
                         [self = shared_from_this()](
                             boost::system::error_code ec, tcp::socket socket) {
                           self->on_accept(ec, std::move(socket));
                         });
}

void HttpServer::on_accept(boost::system::error_code ec, tcp::socket socket) {
  if (ec) {
    if (ec == net::error::operation_aborted) {
      return;
    }
    std::cerr << "Accept error: " << ec.message() << '\n';

    auto timer = std::make_shared<net::steady_timer>(
        acceptor_.get_executor(), std::chrono::milliseconds(100));
    timer->async_wait(
        [self = shared_from_this(), timer](boost::system::error_code ec) {
          if (!ec) {
            self->do_accept();
          }
        });
    return;
  }
  std::make_shared<HttpSession>(std::move(socket), repository_)->run();

  do_accept();
}
}  // namespace Http