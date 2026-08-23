// NOLINTBEGIN
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "db/db_connection_pool.hpp"
#include "db/journal_record.hpp"
#include "db/journal_repository.hpp"
#include "tests.hpp"

inline std::string get_conn_str() {
  const char* env = std::getenv("DB_CONN_STR");
  if (env != nullptr && *env != '\0') {
    return std::string(env);
  }
  return "host=127.0.0.1 port=5432 dbname=journal_db user=postgres "
         "password=postgres";
}

std::int64_t parse_ts(std::string_view ts_str) {
  std::tm tm = {};
  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int min = 0;
  int sec = 0;
  if (sscanf(ts_str.data(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour,
             &min, &sec) >= 6) {
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    std::time_t epoch = timegm(&tm);
    auto tz_pos = ts_str.find_last_of("+-");
    if (tz_pos != std::string_view::npos && tz_pos > 10) {
      char sign = ts_str[tz_pos];
      int tz_hour = 0;
      int tz_min = 0;
      if (sscanf(ts_str.data() + tz_pos + 1, "%d:%d", &tz_hour, &tz_min) >= 1) {
        int offset = tz_hour * 3600 + tz_min * 60;
        epoch += (sign == '+') ? -offset : offset;
      }
    }
    return epoch;
  }
  return 0;
}

bool test_1() {
  try {
    DbConnectionPool pool(get_conn_str(), 2);
    {
      auto guard1 = pool.get_connection();
      ASSERT_TRUE(guard1);
      auto guard2 = pool.get_connection();
      ASSERT_TRUE(guard2);
    }
    auto guard3 = pool.get_connection();
    ASSERT_TRUE(guard3);
    return true;
  } catch (const std::exception& e) {
    std::cerr << "\n  [EXCEPTION] " << e.what() << '\n';
    return false;
  }
}

bool test_2() {
  try {
    DbConnectionPool pool(get_conn_str(), 1);
    JournalRepository repo(pool);
    auto records = repo.get_records(2, "2020-01-01 00:00:00+00", 10);
    ASSERT_TRUE(!records.empty());
    for (const auto& r : records) {
      ASSERT_TRUE(r.level > 2);
    }
    return true;
  } catch (const std::exception& e) {
    std::cerr << "\n  [EXCEPTION] " << e.what() << '\n';
    return false;
  }
}

bool test_3() {
  try {
    DbConnectionPool pool(get_conn_str(), 1);
    JournalRepository repo(pool);
    const std::string since = "2020-01-01 00:00:00+00";
    auto records = repo.get_records(0, since, 10);
    ASSERT_TRUE(!records.empty());
    auto since_epoch = parse_ts(since);
    for (const auto& r : records) {
      ASSERT_TRUE(parse_ts(r.ts) > since_epoch);
    }
    return true;
  } catch (const std::exception& e) {
    std::cerr << "\n  [EXCEPTION] " << e.what() << '\n';
    return false;
  }
}

bool test_4() {
  try {
    DbConnectionPool pool(get_conn_str(), 1);
    JournalRepository repo(pool);
    auto records = repo.get_records(0, "2020-01-01 00:00:00+00", 100);
    ASSERT_TRUE(!records.empty());
    ASSERT_TRUE(records.size() <= 10);

    auto records_3 = repo.get_records(0, "2020-01-01 00:00:00+00", 3);
    ASSERT_TRUE(!records_3.empty());
    ASSERT_TRUE(records_3.size() <= 3);
    return true;
  } catch (const std::exception& e) {
    std::cerr << "\n  [EXCEPTION] " << e.what() << '\n';
    return false;
  }
}

bool test_5() {
  try {
    DbConnectionPool pool(get_conn_str(), 1);
    JournalRepository repo(pool);
    auto records = repo.get_records(1, "2020-01-01 00:00:00+00", 10);
    ASSERT_TRUE(records.size() >= 2);
    for (size_t i = 1; i < records.size(); ++i) {
      ASSERT_TRUE(parse_ts(records[i].ts) >= parse_ts(records[i - 1].ts));
    }
    return true;
  } catch (const std::exception& e) {
    std::cerr << "\n  [EXCEPTION] " << e.what() << '\n';
    return false;
  }
}

int main() {
  int passed = 0;
  int total = 0;

  std::cout << "Интеграционные тесты JournalRepository и DbConnectionPool\n";
  std::cout << "==========================================\n";
  RUN_TEST(test_1);
  RUN_TEST(test_2);
  RUN_TEST(test_3);
  RUN_TEST(test_4);
  RUN_TEST(test_5);

  std::cout << "==========================================\n";
  std::cout << "Итого: " << passed << " из " << total << " тестов пройдено.\n";

  return (passed == total) ? 0 : 1;
}
// NOLINTEND
