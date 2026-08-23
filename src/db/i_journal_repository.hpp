#pragma once
#include <string>
#include <vector>

#include "journal_record.hpp"
class IJournalRepository {
 public:
  virtual ~IJournalRepository() = default;
  virtual std::vector<JournalRecord> get_records(int min_level,
                                                 const std::string& timestamp,
                                                 size_t limit) const = 0;
};
