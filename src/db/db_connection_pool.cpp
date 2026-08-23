#include "db_connection_pool.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

#include "db_connection_guard.hpp"

DbConnectionPool::DbConnectionPool(std::string conn_str, size_t pool_size)
    : conn_str_(std::move(conn_str)), pool_size_(pool_size) {
  pool_.clear();
  for (size_t i = 0; i < pool_size_; ++i) {
    auto conn = std::make_unique<pqxx::connection>(conn_str_);
    if (!conn->is_open()) {
      throw std::runtime_error("Failed to connect to DB");
    }
    pool_.push(std::move(conn));
  }
}
void DbConnectionPool::stop() noexcept {
  pool_.stop();
  pool_.clear();
}
DbConnectionPool::~DbConnectionPool() { stop(); }

DbConnectionGuard DbConnectionPool::get_connection() {
  auto conn = pool_.wait_and_pop();
  if (!conn.has_value()) {
    throw std::runtime_error("Connection pool is stopped");
  }
  if (!(*conn)->is_open()) {
    try {
      conn = std::make_unique<pqxx::connection>(conn_str_);
    } catch (const std::exception& e) {
      throw std::runtime_error("Failed to reconnect to DB");
    }
  }

  return DbConnectionGuard(std::move(*conn), *this);
}
void DbConnectionPool::return_connection(
    std::unique_ptr<pqxx::connection> conn) {
  if (conn && conn->is_open()) {
    pool_.push(std::move(conn));
  } else {
    try {
      auto new_conn = std::make_unique<pqxx::connection>(conn_str_);
      if (new_conn->is_open()) {
        pool_.push(std::move(new_conn));
      }
    } catch (const std::exception& e) {
      std::cerr << "Pool: failed to reconnect: " << e.what() << '\n';
    }
  }
}

DbConnectionGuard::DbConnectionGuard(std::unique_ptr<pqxx::connection> conn,
                                     DbConnectionPool& db_pool)
    : conn_(std::move(conn)), pool_(&db_pool) {}

DbConnectionGuard& DbConnectionGuard::operator=(
    DbConnectionGuard&& other) noexcept {
  if (this != &other) {
    if (conn_ && pool_ != nullptr) {
      pool_->return_connection(std::move(conn_));
    }
    conn_ = std::move(other.conn_);
    pool_ = other.pool_;
    other.pool_ = nullptr;
  }
  return *this;
}

pqxx::connection& DbConnectionGuard::get() { return *conn_; }
const pqxx::connection& DbConnectionGuard::get() const { return *conn_; }

pqxx::connection* DbConnectionGuard::operator->() { return conn_.get(); }
const pqxx::connection* DbConnectionGuard::operator->() const {
  return conn_.get();
}

DbConnectionGuard::operator bool() const noexcept { return conn_ != nullptr; }

DbConnectionGuard::DbConnectionGuard(DbConnectionGuard&& other) noexcept
    : conn_(std::move(other.conn_)), pool_(other.pool_) {
  other.pool_ = nullptr;
}
DbConnectionGuard::~DbConnectionGuard() {
  if (conn_ && pool_ != nullptr) {
    pool_->return_connection(std::move(conn_));
  }
}