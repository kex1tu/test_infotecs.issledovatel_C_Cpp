#pragma once

#include <string>
#include <vector>

#include "db_connection_pool.hpp"
#include "i_journal_repository.hpp"
#include "journal_record.hpp"
class JournalRepository : public IJournalRepository {
 public:
  explicit JournalRepository(DbConnectionPool& pool);
  std::vector<JournalRecord> get_records(
      int min_level = 0,
      const std::string& timestamp = "1970-01-01 00:00:00+00",
      size_t limit = 10) const override;

 private:
  DbConnectionPool& pool_;
};