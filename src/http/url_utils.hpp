#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace Http {

std::string url_decode(std::string_view in);
std::unordered_map<std::string, std::string> parse_query_string(
    std::string_view query_string);
std::pair<std::string_view, std::string_view> split_target(
    std::string_view target);

}  // namespace Http