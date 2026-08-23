#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/system/detail/error_code.hpp>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "db/db_connection_pool.hpp"
#include "db/journal_repository.hpp"
#include "http/http_server.hpp"

namespace net = boost::asio;
using tcp = net::ip::tcp;

int main(int argc, char** argv) {
  try {
    if (argc < 3) {
      std::cerr << "Usage: " << argv[0] << " <host> <port> [<db_conn_str>]\n";
      return 1;
    }
    const std::string host = argv[1];
    int port_raw = std::stoi(argv[2]);
    if (port_raw < 0 || port_raw > 65535) {
      std::cerr << "Port must be in range [0, 65535]\n";
      return 1;
    }
    uint16_t port = static_cast<uint16_t>(port_raw);
    const int kThreadCount = 4;

    std::cout << "Starting Journal Service on " << host << ":" << port
              << "...\n";
    std::string conn_str =
        "host=127.0.0.1 port=5432 dbname=journal_db user=postgres "
        "password=postgres";
    if (argc > 3) {
      conn_str = argv[3];
    } else if (const char* conn_env = std::getenv("DB_CONN_STR");
               conn_env != nullptr && *conn_env != '\0') {
      conn_str = conn_env;
    }

    DbConnectionPool db_pool(conn_str, kThreadCount);
    JournalRepository repository(db_pool);

    net::io_context worker_ioc{kThreadCount};
    net::io_context acceptor_ioc{1};

    auto worker_work_guard = net::make_work_guard(worker_ioc);
    auto acceptor_work_guard = net::make_work_guard(acceptor_ioc);

    auto const address = net::ip::make_address(host);
    auto server = std::make_shared<Http::HttpServer>(
        acceptor_ioc, worker_ioc, tcp::endpoint{address, port}, repository);
    server->run();

    net::signal_set signals(acceptor_ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&server, &db_pool, &worker_work_guard, &acceptor_work_guard](
            const boost::system::error_code ec, int signal_number) {
          std::cout << "Received shutdown signal (" << signal_number
                    << "). Stopping server...\n";
          server->stop();
          db_pool.stop();
          acceptor_work_guard.reset();
          worker_work_guard.reset();
        });

    std::vector<std::thread> workers;
    workers.reserve(kThreadCount);
    for (int i = 0; i < kThreadCount; ++i) {
      workers.emplace_back(std::thread([&worker_ioc]() { worker_ioc.run(); }));
    }
    std::thread acceptor_thread([&acceptor_ioc]() { acceptor_ioc.run(); });

    std::cout << "Journal Web Service is running with " << kThreadCount
              << " worker threads. Press Ctrl+C to stop.\n";
    if (acceptor_thread.joinable()) {
      acceptor_thread.join();
    }
    for (auto& worker : workers) {
      if (worker.joinable()) {
        worker.join();
      }
    }

    std::cout << "Server stopped gracefully.\n";

  } catch (std::exception const& e) {
    std::cerr << "Exception: " << e.what() << '\n';
  }
  return 0;
}