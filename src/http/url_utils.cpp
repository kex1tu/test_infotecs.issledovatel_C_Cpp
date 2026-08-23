#include "url_utils.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace Http {

std::pair<std::string_view, std::string_view> split_target(
    std::string_view target) {
  std::size_t q_pos = target.find('?');
  if (q_pos == std::string_view::npos) {
    return {target, {}};
  }
  return {target.substr(0, q_pos), target.substr(q_pos + 1)};
}
namespace {

auto hex_value = [](char c) -> int {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
};

}  // namespace
std::string url_decode(std::string_view in) {
  std::string res;
  res.reserve(in.size());
  size_t idx = 0;
  while (idx < in.size()) {
    if (in[idx] == '%') {
      if (idx + 2 < in.size()) {
        int v1 = hex_value(in[idx + 1]);
        int v2 = hex_value(in[idx + 2]);
        if (v1 != -1 && v2 != -1) {
          res += static_cast<char>((v1 * 16) + v2);
          idx += 3;
          continue;
        }
      }
      res += in[idx];
    } else if (in[idx] == '+') {
      res += ' ';
    } else {
      res += in[idx];
    }
    ++idx;
  }
  return res;
}
std::unordered_map<std::string, std::string> parse_query_string(
    std::string_view query_string) {
  std::unordered_map<std::string, std::string> res;
  size_t start = 0;
  while (start < query_string.size()) {
    size_t end = query_string.find('&', start);
    if (end == std::string_view::npos) {
      end = query_string.size();
    }
    std::string_view pair = query_string.substr(start, end - start);
    size_t eq_pos = pair.find('=');
    if (eq_pos != std::string_view::npos) {
      std::string_view key = pair.substr(0, eq_pos);
      std::string_view value = pair.substr(eq_pos + 1);
      if (!key.empty()) {
        res[url_decode(key)] = url_decode(value);
      }
    } else if (!pair.empty()) {
      res[url_decode(pair)] = "";
    }
    start = end + 1;
  }
  return res;
}
}  // namespace Http