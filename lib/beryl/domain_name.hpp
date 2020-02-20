#pragma once

#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <boost/container/string.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace beryl {
class label_error : public std::runtime_error {
public:
  explicit label_error(const std::string_view& label)
      : std::runtime_error(std::string("invalid label: `") + label.data() +
                           "`") {}
};
class domain_name_error : public std::runtime_error {
public:
  explicit domain_name_error(const std::string_view& domain_name)
      : std::runtime_error(std::string("invalid domain name: `") +
                           domain_name.data() + "`") {}
};

namespace _impl {
template <typename T>
class domain_name_extender;
}

class label_view {
public:
  class passkey {
  private:
    friend class label;
    template <typename T>
    friend class _impl::domain_name_extender;
    constexpr passkey() noexcept = default;
  };

  explicit label_view(const std::string_view& l);
  explicit label_view(const char* c) : label_view(std::string_view(c)) {}
  label_view(const char* c, std::size_t size)
      : label_view(std::string_view(c, size)) {}
  constexpr label_view(const char* c, std::size_t size,
                       [[maybe_unused]] passkey key)
      : _label(c, size) {}

  [[nodiscard]] const char* data() const noexcept { return _label.data(); }
  [[nodiscard]] std::size_t size() const noexcept { return _label.size(); }

  friend bool
  operator==(const label_view& lhs, const label_view& rhs) noexcept {
    return lhs._label == rhs._label;
  }
  friend bool operator<(const label_view& lhs, const label_view& rhs) noexcept {
    return lhs._label < rhs._label;
  }
  friend std::ostream& operator<<(std::ostream& os, const label_view& l) {
    os << l._label;
    return os;
  }

private:
  std::string_view _label;
};

class label {
public:
  explicit label(const label_view& l) : _label(l.data(), l.size()) {}
  explicit label(const std::string_view& l) : label(label_view(l)) {}
  explicit label(const char* c) : label(label_view(c)) {}
  label(const char* c, std::size_t size) : label(label_view(c, size)) {}

  [[nodiscard]] const char* data() const noexcept { return _label.data(); }
  [[nodiscard]] std::size_t size() const noexcept { return _label.size(); }

  operator label_view() const noexcept {
    return label_view(_label.data(), _label.size(), label_view::passkey());
  }

  friend bool operator==(const label& lhs, const label& rhs) noexcept {
    return lhs._label == rhs._label;
  }
  friend bool operator<(const label& lhs, const label& rhs) noexcept {
    return lhs._label < rhs._label;
  }
  friend std::ostream& operator<<(std::ostream& os, const label& l) {
    os << l._label;
    return os;
  }

private:
  boost::container::string _label;
};

namespace _impl {
template <typename T>
class domain_name_extender {
public:
  class const_iterator
      : public boost::iterator_facade<const_iterator, const label_view,
                                      boost::forward_traversal_tag> {
  public:
    [[nodiscard]] const label_view& dereference() const noexcept {
      return _label;
    }
    [[nodiscard]] bool equal(const const_iterator& other) const noexcept {
      return _label == other._label;
    }
    void increment() noexcept {
      const char* data = _label.data();
      std::size_t size = _label.size();
      // @note. Please see comment for domain_name_extender::end method.
      auto new_size = static_cast<std::size_t>(-*(data + size));
      _label = label_view(data + size + 1, new_size, label_view::passkey());
    }

  private:
    friend class domain_name_extender;
    friend class boost::iterator_core_access;

    const_iterator(const char* c, std::size_t s)
        : _label(c, s, label_view::passkey()) {}

    label_view _label;
  };

  [[nodiscard]] const_iterator begin() const noexcept {
    const char* pos = static_cast<const T*>(this)->_dname.data();
    return const_iterator(pos + 1, static_cast<std::size_t>(-*pos));
  }
  [[nodiscard]] const_iterator end() const noexcept {
    // @note. Techically, `pos` point to the null `\0` terminating the `_dname`
    // string whereas `pos + 1` points beyond the valid range. Howwever,
    // the latter does not result to an undefined behavior because
    // no data is read from `pos + 1` address.
    const char* pos = static_cast<const T*>(this)->_dname.data() +
                      static_cast<const T*>(this)->_dname.size();
    return const_iterator(pos + 1, static_cast<std::size_t>(-*pos));
  }

  bool operator==(const T& other) const noexcept {
    return static_cast<const T*>(this)->_dname ==
           static_cast<const T*>(&other)->_dname;
  }
  bool operator!=(const T& other) const noexcept {
    return !(*static_cast<const T*>(this) == *static_cast<const T*>(&other));
  }
};
}  // namespace _impl

class domain_name : public _impl::domain_name_extender<domain_name> {
public:
  // @param str - a fully qualified domain name, must end with dot.
  //     Example: `eecs.berkeley.edu.`
  //
  // @throw domain_name_error if str is not a valid domain name
  explicit domain_name(const std::string_view& str);

  // // @param bytes - a fully qualified domain name in the format of DNS
  // message
  // //     protocol. Example: `0x0465656373086265726b656c65790365647500`
  // //     for `eecs.berkeley.edu.`
  // //
  // // @throw domain_name_error if str is not a valid domain name
  // domain_name(const std::vector<std::byte>& bytes);

  domain_name& remove_subdomain() {
    boost::container::string::const_reverse_iterator delim = std::find_if(
        _dname.crbegin(), _dname.crend(), [](char c) { return c < 0; });
    if (delim != _dname.crend()) {
      ++delim;
    }
    _dname.erase(delim.base(), _dname.cend());
    return *this;
  }
  domain_name& add_subdomain(const label_view& l) {
    _dname.append(1, static_cast<char>(-1 * static_cast<char>(l.size())))
        .append(l.data(), l.size());
    return *this;
  }

private:
  friend class domain_name_extender<domain_name>;

  boost::container::string _dname;
};

class domain_name_view : public _impl::domain_name_extender<domain_name_view> {
public:
  domain_name_view(domain_name::const_iterator begin,
                   domain_name::const_iterator end) noexcept
      : _dname(begin->data() - 1,
               static_cast<std::size_t>(end->data() - (begin->data() - 1))) {}
  explicit domain_name_view(const domain_name& dname) noexcept
      : domain_name_view(dname.begin(), dname.end()) {}
  domain_name_view(const domain_name& dname, std::size_t label_count) noexcept
      : domain_name_view(
            dname.begin(),
            std::next(dname.begin(),
                      static_cast<typename std::iterator_traits<
                          const_iterator>::difference_type>(label_count))) {}

private:
  friend class domain_name_extender<domain_name_view>;

  std::string_view _dname;
};

// mostly for testing and debug purposes
template <typename T>
std::enable_if_t<std::is_same_v<T, domain_name> ||
                     std::is_same_v<T, domain_name_view>,
                 std::ostream&>
operator<<(std::ostream& os, const T& dname) {
  bool empty = true;
  for (const auto& label : dname) {
    os << "." << label;
    empty = false;
  }
  if (empty) {
    os << ".";
  }
  return os;
}
template <typename T>
std::enable_if_t<std::is_same_v<T, domain_name> ||
                     std::is_same_v<T, domain_name_view>,
                 std::string>
to_string(const T& dname) {
  std::ostringstream s;
  s << dname;
  return s.str();
}
}  // namespace beryl
