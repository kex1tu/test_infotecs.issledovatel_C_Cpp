#include "http/json_serializer.hpp"

#include <boost/json.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>

namespace Http {
std::string serialize_records_to_json(
    const std::vector<JournalRecord>& records) {
  boost::json::array arr;
  arr.reserve(records.size());
  for (const auto& record : records) {
    boost::json::object obj;
    obj["id"] = record.id;
    obj["ts"] = record.ts;
    obj["level"] = record.level;
    obj["message"] = record.message;
    arr.push_back(std::move(obj));
  }
  return boost::json::serialize(arr);
}
std::string serialize_error_to_json(std::string_view error_message) {
  boost::json::object obj;
  obj["error"] = error_message;
  return boost::json::serialize(obj);
}

}  // namespace Http