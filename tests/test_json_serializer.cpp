// NOLINTBEGIN
#include <boost/json.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "db/journal_record.hpp"
#include "http/json_serializer.hpp"
#include "tests.hpp"

bool test_1() {
  std::vector<JournalRecord> records;
  std::string json = Http::serialize_records_to_json(records);
  ASSERT_EQ(json, "[]");
  return true;
}

bool test_2() {
  std::vector<JournalRecord> records = {{.id = 42,
                                         .ts = "2025-08-22 14:11:13+00",
                                         .level = 3,
                                         .message = "Server started"}};

  std::string json = Http::serialize_records_to_json(records);
  boost::json::value val = boost::json::parse(json);
  ASSERT_TRUE(val.is_array());
  auto arr = val.as_array();
  ASSERT_EQ(arr.size(), 1);
  ASSERT_TRUE(arr[0].is_object());
  auto obj = arr[0].as_object();
  ASSERT_TRUE(obj.contains("id") && obj.contains("ts") &&
              obj.contains("level") && obj.contains("message"));
  ASSERT_EQ(obj.at("id").as_int64(), 42);
  ASSERT_EQ(obj.at("ts").as_string(), "2025-08-22 14:11:13+00");
  ASSERT_EQ(obj.at("level").as_int64(), 3);
  ASSERT_EQ(obj.at("message").as_string(), "Server started");
  return true;
}

bool test_3() {
  std::vector<JournalRecord> records = {
      {.id = 1, .ts = "2025-01-01", .level = 1, .message = "Msg 1"},
      {.id = 2, .ts = "2025-01-02", .level = 2, .message = "Msg 2"}};

  std::string json = Http::serialize_records_to_json(records);
  boost::json::value val = boost::json::parse(json);
  ASSERT_TRUE(val.is_array());
  auto arr = val.as_array();
  ASSERT_EQ(arr.size(), 2);
  ASSERT_TRUE(arr[0].is_object() && arr[1].is_object());
  ASSERT_EQ(arr[0].as_object().at("id").as_int64(), 1);
  ASSERT_EQ(arr[1].as_object().at("id").as_int64(), 2);
  return true;
}

bool test_4() {
  std::vector<JournalRecord> records = {
      {.id = 10,
       .ts = "2025-08-22",
       .level = 4,
       .message = "ERROR: \"Test\"\nTest 1\tTest 2"}};

  std::string json = Http::serialize_records_to_json(records);
  boost::json::value val = boost::json::parse(json);
  ASSERT_TRUE(val.is_array());
  ASSERT_TRUE(!val.as_array().empty());
  ASSERT_TRUE(val.as_array()[0].is_object());
  auto obj = val.as_array()[0].as_object();
  ASSERT_TRUE(obj.contains("message"));
  ASSERT_EQ(obj.at("message").as_string(), "ERROR: \"Test\"\nTest 1\tTest 2");
  return true;
}

bool test_5() {
  std::string json =
      Http::serialize_error_to_json("Missing required parameter: 'since'");
  boost::json::value val = boost::json::parse(json);
  ASSERT_TRUE(val.is_object());
  ASSERT_TRUE(val.as_object().contains("error"));
  ASSERT_EQ(val.as_object().at("error").as_string(),
            "Missing required parameter: 'since'");
  return true;
}

int main() {
  int passed = 0;
  int total = 0;

  std::cout << "Тесты JsonSerializer\n";
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
