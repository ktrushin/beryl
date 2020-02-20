#pragma once

#include <cassert>

#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace beryl {
enum class record_type : std::uint8_t {
  a = 1,
  ns = 2,
  cname = 5,
  soa = 6,
  aaaa = 28
};

inline std::string to_string(record_type t) {
  switch (t) {
    case record_type::a: return "A";
    case record_type::ns: return "NS";
    case record_type::cname: return "CNAME";
    case record_type::soa: return "SOA";
    case record_type::aaaa: return "AAAA";
  }
  assert(false && "unexpected resource record type");
}

inline std::ostream& operator<<(std::ostream& os, record_type rt) {
  os << to_string(rt);
  return os;
}

inline record_type to_record_type(const std::string& str) {
  static const std::unordered_map<std::string, record_type> m = {
      {to_string(record_type::a), record_type::a},
      {to_string(record_type::ns), record_type::ns},
      {to_string(record_type::cname), record_type::cname},
      {to_string(record_type::soa), record_type::soa},
      {to_string(record_type::aaaa), record_type::aaaa}};
  if (auto it = m.find(str); it != m.end()) {
    return it->second;
  }
  throw std::runtime_error("unsupported resource record type: `" + str + "`");
}
}  // namespace beryl
