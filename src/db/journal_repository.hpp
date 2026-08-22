#pragma once

#include <string>
#include <vector>

#include "db_connection_pool.hpp"
#include "journal_record.hpp"
class JournalRepository {
 public:
  explicit JournalRepository(DbConnectionPool& pool);
  std::vector<JournalRecord> get_records(int min_level,
                                         const std::string& timestamp,
                                         size_t limit = 10);

 private:
  DbConnectionPool& pool_;
};