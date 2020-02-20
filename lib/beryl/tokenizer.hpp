#pragma once

#include <cassert>

#include <string_view>

#include <boost/iterator/iterator_facade.hpp>

namespace beryl {
class tokenizer {
private:
  class passkey {
  private:
    friend class tokenizer;
    passkey() noexcept = default;
  };

public:
  class const_iterator
      : public boost::iterator_facade<const_iterator, const std::string_view,
                                      boost::forward_traversal_tag> {
  public:
    const_iterator(const tokenizer* tokenizer,
                   std::string_view::size_type position,
                   [[maybe_unused]] passkey key)
        : _tkzr((assert(tokenizer && "must not be a nullptr"), tokenizer)),
          _pos(position) {
      fetch_token();
    }

  private:
    friend class boost::iterator_core_access;

    [[nodiscard]] const std::string_view& dereference() const noexcept {
      return _token;
    }
    void increment() noexcept {
      _pos = _delim_pos == std::string_view::npos ? std::string_view::npos
                                                  : _pos + _token.size() + 1;
      fetch_token();
    }
    [[nodiscard]] bool equal(const const_iterator& other) const noexcept {
      return _tkzr == other._tkzr && _pos == other._pos;
    }
    void fetch_token() noexcept {
      if (_pos != std::string_view::npos) {
        _delim_pos = _tkzr->_str.find(_tkzr->_delim, _pos);
        _token = _tkzr->_str.substr(_pos, _delim_pos - _pos);
      }
    }

    const tokenizer* _tkzr;
    std::string_view::size_type _pos;
    std::string_view::size_type _delim_pos;
    std::string_view _token;
  };

  tokenizer(const std::string_view& string, char delimiter) noexcept
      : _str(string), _delim(delimiter) {}

  [[nodiscard]] const_iterator begin() const noexcept {
    return const_iterator(this, 0, passkey());
  }
  [[nodiscard]] const_iterator end() const noexcept {
    return const_iterator(this, std::string_view::npos, passkey());
  }

private:
  friend class const_iterator;

  std::string_view _str;
  char _delim;
};
}  // namespace beryl
