#include "beryl/domain_name.hpp"

#include "beryl/string.hpp"
#include "beryl/tokenizer.hpp"

namespace beryl {
namespace {
constexpr std::size_t max_label_length = 63;
constexpr std::size_t max_domain_name_length = 255;
bool is_alnum(char c) noexcept {
  return std::isalnum(static_cast<unsigned char>(c)) != 0;
}

bool str_interior_chars_are_alnums_or_hyphens(
    const std::string_view& str) noexcept {
  for (std::size_t i = 1; i + 1 < str.size(); ++i) {
    if (!(is_alnum(str[i]) || str[i] == '-')) {
      return false;
    }
  }
  return true;
}

bool label_is_valid(const std::string_view& label) noexcept {
  return !label.empty() && label.size() <= max_label_length &&
         is_alnum(label.front()) && is_alnum(label.back()) &&
         str_interior_chars_are_alnums_or_hyphens(label);
}
}  // namespace

label_view::label_view(const std::string_view& l)
    : _label(label_is_valid(l) ? l : throw label_error(l)) {}

domain_name::domain_name(const std::string_view& str) {
  static constexpr std::string_view root = ".";
  if (str == root) {
    _dname = boost::container::string();
    return;
  }
  if (!(str.size() <= max_domain_name_length && str_ends_with(str, '.'))) {
    throw domain_name_error(str);
  }

  _dname = boost::container::string(str.size(), '#');
  std::size_t pos = _dname.size();
  // @note. Ignore the trailing dot in the fully qualified domain name.
  // Otherwise, `labels` yields the final empty label which is not needed.
  tokenizer labels(std::string_view(str.data(), str.size() - 1), '.');
  for (const auto& label : labels) {
    if (!label_is_valid(label)) {
      throw domain_name_error(str);
    }
    pos -= label.size() + 1;
    _dname[pos] = static_cast<char>(-1 * static_cast<char>(label.size()));
    _dname.replace(pos + 1, label.size(), label);
  }

  for (auto& c : _dname) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
}

}  // namespace beryl
