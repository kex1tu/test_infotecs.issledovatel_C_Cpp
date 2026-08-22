#include <iostream>

#include "db/db_connection_pool.hpp"
#include "db/journal_repository.hpp"

int main() {
  std::string conn_str =
      "host=127.0.0.1 port=5432 dbname=journal_db user=postgres "
      "password=postgres";
  DbConnectionPool pool(conn_str, 4);
  JournalRepository repo(pool);
  try {
    auto results = repo.get_records(1, "2025-01-01 00:00:00+00", 10);

    std::cout << "Get " << results.size() << " records: \n";
    for (auto const& record : results) {
      std::cout << "ID: " << record.id << " TS: " << record.ts
                << " Level: " << record.level << " Message: " << record.message
                << '\n';
    }
  } catch (std::exception& e) {
    std::cerr << "Error " << e.what() << '\n';
  }
  return 0;
}