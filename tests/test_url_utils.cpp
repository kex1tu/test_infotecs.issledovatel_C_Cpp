
// NOLINTBEGIN
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

#include "http/url_utils.hpp"
#include "tests.hpp"

bool test_1() {
  ASSERT_EQ(Http::url_decode("hello"), "hello");
  ASSERT_EQ(Http::url_decode("hello+world"), "hello world");
  ASSERT_EQ(Http::url_decode("hello%20world"), "hello world");
  ASSERT_EQ(Http::url_decode("a+b%20c+d"), "a b c d");
  return true;
}

bool test_2() {
  ASSERT_EQ(Http::url_decode("2025-08-22%2014%3A11%3A13%2B00"),
            "2025-08-22 14:11:13+00");
  ASSERT_EQ(Http::url_decode("2025-08-22%2017%3A11%3A13%2B03"),
            "2025-08-22 17:11:13+03");
  return true;
}

bool test_3() {
  ASSERT_EQ(Http::url_decode("%2b"), "+");
  ASSERT_EQ(Http::url_decode("%2B"), "+");
  ASSERT_EQ(Http::url_decode("%3a"), ":");
  ASSERT_EQ(Http::url_decode("%3A"), ":");
  return true;
}

bool test_4() {
  ASSERT_EQ(Http::url_decode(""), "");
  ASSERT_EQ(Http::url_decode("%"), "%");
  ASSERT_EQ(Http::url_decode("%2"), "%2");
  ASSERT_EQ(Http::url_decode("%ZZ"), "%ZZ");
  ASSERT_EQ(Http::url_decode("100%_complete"), "100%_complete");
  return true;
}

bool test_5() {
  auto [path, query] = Http::split_target("/journal?since=2020-01-01&level=2");
  ASSERT_EQ(path, "/journal");
  ASSERT_EQ(query, "since=2020-01-01&level=2");
  return true;
}

bool test_6() {
  auto [path, query] = Http::split_target("/journal");
  ASSERT_EQ(path, "/journal");
  ASSERT_EQ(query, "");
  return true;
}

bool test_7() {
  auto [path, query] = Http::split_target("/journal?");
  ASSERT_EQ(path, "/journal");
  ASSERT_EQ(query, "");
  return true;
}

bool test_8() {
  auto params = Http::parse_query_string("since=2025-08-22");
  ASSERT_EQ(params.size(), 1);
  ASSERT_EQ(params["since"], "2025-08-22");
  return true;
}

bool test_9() {
  auto params = Http::parse_query_string("since=2025-08-22&level=3");
  ASSERT_EQ(params.size(), 2);
  ASSERT_EQ(params["since"], "2025-08-22");
  ASSERT_EQ(params["level"], "3");
  return true;
}

bool test_10() {
  auto params =
      Http::parse_query_string("since=2025-08-22%2014%3A11%3A13%2B00&level=2");
  ASSERT_EQ(params.size(), 2);
  ASSERT_EQ(params["since"], "2025-08-22 14:11:13+00");
  ASSERT_EQ(params["level"], "2");
  return true;
}

bool test_11() {
  auto params = Http::parse_query_string("");
  ASSERT_TRUE(params.empty());
  return true;
}

bool test_12() {
  auto params = Http::parse_query_string("flag&key=&other=val");
  ASSERT_EQ(params.size(), 3);
  ASSERT_EQ(params["flag"], "");
  ASSERT_EQ(params["key"], "");
  ASSERT_EQ(params["other"], "val");
  return true;
}

int main() {
  int passed = 0;
  int total = 0;

  std::cout << "Тесты UrlUtils\n";
  std::cout << "==========================================\n";
  RUN_TEST(test_1);
  RUN_TEST(test_2);
  RUN_TEST(test_3);
  RUN_TEST(test_4);
  RUN_TEST(test_5);
  RUN_TEST(test_6);
  RUN_TEST(test_7);
  RUN_TEST(test_8);
  RUN_TEST(test_9);
  RUN_TEST(test_10);
  RUN_TEST(test_11);
  RUN_TEST(test_12);

  std::cout << "==========================================\n";
  std::cout << "Итого: " << passed << " из " << total << " тестов пройдено.\n";

  return (passed == total) ? 0 : 1;
}
// NOLINTEND
