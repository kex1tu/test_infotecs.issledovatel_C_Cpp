#include "journal_repository.hpp"

#include <iostream>
#include <pqxx/pqxx>
#include <vector>

#include "journal_record.hpp"

JournalRepository::JournalRepository(DbConnectionPool& pool) : pool_(pool) {}

std::vector<JournalRecord> JournalRepository::get_records(
    int min_level, const std::string& timestamp, size_t limit) const {
  limit = std::min(limit, static_cast<size_t>(10));
  std::vector<JournalRecord> records;
  for (int attempt = 0; attempt < 2; ++attempt) {
    try {
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
                          .level = row["level"].as<int>(),
                          .message = row["message"].as<std::string>()});
      }
      return records;
    } catch (const pqxx::broken_connection& e) {
      std::cerr << "DB connection broken: " << e.what() << " (attempt "
                << attempt + 1 << "/2)\n";
      if (attempt == 1) {
        throw;
      }
    } catch (const std::exception& e) {
      std::cerr << "DB query error: " << e.what() << '\n';
      throw;
    }
  }
  return {};
}
