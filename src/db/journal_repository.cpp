#include "journal_repository.hpp"

#include <pqxx/pqxx>
#include <vector>

#include "journal_record.hpp"

JournalRepository::JournalRepository(DbConnectionPool& pool) : pool_(pool) {}

std::vector<JournalRecord> JournalRepository::get_records(
    int min_level, const std::string& timestamp, size_t limit) {
  std::vector<JournalRecord> records;

  auto guard = pool_.get_connection();
  pqxx::read_transaction rtrans(guard.get());
  pqxx::result res = rtrans.exec_params(
      "SELECT id, ts, message, level "
      "FROM journal "
      "WHERE level > $1 AND ts > $2 "
      "ORDER BY ts ASC "
      "LIMIT $3",
      min_level, timestamp, limit);
  records.reserve(res.size());
  for (const auto& row : res) {
    records.push_back(
        JournalRecord{.id = row["id"].as<int64_t>(),
                      .ts = row["ts"].as<std::string>(),
                      .level = static_cast<uint8_t>(row["level"].as<int>()),
                      .message = row["message"].as<std::string>()});
  }

  return records;
};
