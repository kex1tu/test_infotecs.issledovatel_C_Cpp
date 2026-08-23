#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "db/journal_record.hpp"

namespace Http {

std::string serialize_records_to_json(
    const std::vector<JournalRecord>& records);

std::string serialize_error_to_json(std::string_view error_message);

}  // namespace Http