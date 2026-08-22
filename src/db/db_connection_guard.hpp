#pragma once

#include <memory>
#include <pqxx/pqxx>

class DbConnectionPool;

class DbConnectionGuard {
 public:
  DbConnectionGuard(std::unique_ptr<pqxx::connection> conn,
                    DbConnectionPool& db_pool);
  DbConnectionGuard(const DbConnectionGuard&) = delete;
  DbConnectionGuard(DbConnectionGuard&&) noexcept;
  DbConnectionGuard& operator=(const DbConnectionGuard&) = delete;
  DbConnectionGuard& operator=(DbConnectionGuard&&) noexcept;
  ~DbConnectionGuard();

  pqxx::connection& get();
  const pqxx::connection& get() const;

  pqxx::connection* operator->();
  const pqxx::connection* operator->() const;

  explicit operator bool() const noexcept;

 private:
  std::unique_ptr<pqxx::connection> conn_;
  DbConnectionPool* pool_{nullptr};
};
