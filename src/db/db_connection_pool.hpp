#pragma once
#include <memory>
#include <pqxx/pqxx>

#include "db_connection_guard.hpp"
#include "thread_safe_queue.hpp"

class DbConnectionPool {
  friend class DbConnectionGuard;

 public:
  DbConnectionPool(std::string conn_str, size_t pool_size);
  DbConnectionPool(const DbConnectionPool&) = delete;
  DbConnectionPool& operator=(const DbConnectionPool&) = delete;
  DbConnectionPool(DbConnectionPool&&) = delete;
  DbConnectionPool& operator=(DbConnectionPool&&) = delete;
  ~DbConnectionPool();
  DbConnectionGuard get_connection();
  void stop() noexcept;

 private:
  void return_connection(std::unique_ptr<pqxx::connection> conn);
  std::string conn_str_;
  size_t pool_size_;
  Helpers::ThreadSafeQueue<std::unique_ptr<pqxx::connection>> pool_;
};