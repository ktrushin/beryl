#pragma once

#include <cassert>

#include <ostream>
#include <stdexcept>
#include <string>

namespace beryl {
enum class record_class : std::uint8_t { in = 1 };

inline std::string to_string(record_class c) {
  if (c != record_class::in) {
    assert(false && "unexpected resource record class");
  }
  return "IN";
}

inline std::ostream& operator<<(std::ostream& os, record_class rc) {
  os << to_string(rc);
  return os;
}

inline record_class to_record_class(const std::string& str) {
  if (str != to_string(record_class::in)) {
    // clang-format off
    throw std::runtime_error(
        "unsupported resource record class: `" + str + "`");
    // clang-format on
  }
  return record_class::in;
}
}  // namespace beryl
