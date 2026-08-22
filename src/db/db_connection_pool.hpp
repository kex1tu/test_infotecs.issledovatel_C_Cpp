#pragma once
#include <memory>
#include <pqxx/pqxx>

#include "db_connection_guard.hpp"
#include "thread_safe_queue.hpp"

class DbConnectionPool {
 public:
  DbConnectionPool(std::string conn_str, size_t pool_size);
  ~DbConnectionPool();
  DbConnectionGuard get_connection();
  void return_connection(std::unique_ptr<pqxx::connection> conn);
  void stop() noexcept;

 private:
  std::string conn_str_;
  size_t pool_size_;
  Helpers::ThreadSafeQueue<std::unique_ptr<pqxx::connection>> pool_;
};