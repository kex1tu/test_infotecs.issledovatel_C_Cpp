#pragma once
#include <cstdint>
#include <string>

struct JournalRecord {
  std::int64_t id{0};
  std::string ts;
  int level{0};
  std::string message;
};