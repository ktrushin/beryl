#pragma once

#include <string_view>

#include <boost/container/string.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace beryl {

class boost_string_hash {
public:
  std::size_t operator()(const boost::container::string& str) const noexcept {
    return _h(std::string_view(str));
  }

private:
  std::hash<std::string_view> _h;
};

inline bool str_starts_with(const std::string_view& sv, char c) {
  return !sv.empty() && sv[0] == c;
}
inline bool str_starts_with(const std::string& str, char c) {
  return str_starts_with(std::string_view(str), c);
}
inline bool str_starts_with(const boost::container::string& str, char c) {
  return str_starts_with(std::string_view(str), c);
}

inline bool str_ends_with(const std::string_view& sv, char c) {
  return !sv.empty() && sv[sv.size() - 1] == c;
}
inline bool str_ends_with(const std::string& str, char c) {
  return str_ends_with(std::string_view(str), c);
}
inline bool str_ends_with(const boost::container::string& str, char c) {
  return str_ends_with(std::string_view(str), c);
}

inline bool
str_starts_with(const std::string_view& sv, const std::string_view& prefix) {
  return sv.substr(0, prefix.size()) == prefix;
}
inline bool str_starts_with(const std::string& str, const std::string& prefix) {
  return str_starts_with(std::string_view(str), std::string_view(prefix));
}
inline bool str_starts_with(const boost::container::string& str,
                            const boost::container::string& prefix) {
  return str_starts_with(std::string_view(str), std::string_view(prefix));
}

inline bool
str_ends_with(const std::string_view& sv, const std::string_view& suffix) {
  return suffix.size() <= sv.size() &&
         sv.substr(sv.size() - suffix.size(), suffix.size()) == suffix;
}
inline bool str_ends_with(const std::string& str, const std::string& suffix) {
  return str_ends_with(std::string_view(str), std::string_view(suffix));
}
inline bool str_ends_with(const boost::container::string& str,
                          const boost::container::string& suffix) {
  return str_ends_with(std::string_view(str), std::string_view(suffix));
}
}  // namespace beryl
