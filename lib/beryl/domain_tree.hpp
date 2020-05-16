#pragma once

#include <cassert>

#include <algorithm>
#include <forward_list>
#include <iterator>
#include <utility>
#include <vector>

#include "beryl/domain_name.hpp"

namespace beryl {

// @note. This data structure is far from optimal from both performance and
// memory standpoints but the only practical one to implement given
// a limited time frame. A radix tree would be much more suitable here.
// @todo Evaluate https://github.com/antirez/rax and _adapt_ it
// to the usecase of `domain_tree`.
template <typename T>
class domain_tree {
public:
  using value_type = T;

private:
  class node {
  private:
    using node_container_type =
        std::vector<std::pair<label, std::unique_ptr<node>>>;
    using value_container_type = std::forward_list<value_type>;

  public:
    using iterator = typename node_container_type::iterator;
    using const_iterator = typename node_container_type::const_iterator;
    using value_iterator = typename value_container_type::iterator;
    using const_value_iterator = typename value_container_type::const_iterator;

    [[nodiscard]] bool children_empty() const noexcept {
      return _children.empty();
    }
    iterator children_begin() noexcept { return _children.begin(); }
    iterator children_end() noexcept { return _children.end(); }
    const_iterator children_begin() const noexcept { return _children.begin(); }
    const_iterator children_end() const noexcept { return _children.end(); }

    [[nodiscard]] bool values_empty() const noexcept { return _values.empty(); }
    value_iterator values_begin() noexcept { return _values.begin(); }
    value_iterator values_end() noexcept { return _values.end(); }
    const_value_iterator values_begin() const noexcept {
      return _values.begin();
    }
    const_value_iterator values_end() const noexcept { return _values.end(); }

    iterator find(const label_view& l) noexcept {
      if (auto pos = find_child_insert_pos(l);
          pos != children_end() && pos->first == l) {
        return pos;
      }
      return children_end();
    }
    const_iterator find(const label_view& l) const noexcept {
      if (auto pos = find_child_insert_pos(l);
          pos != children_end() && pos->first == l) {
        return pos;
      }
      return children_end();
    }
    std::pair<iterator, bool> insert_child(const label_view& l) {
      auto pos = find_child_insert_pos(l);
      return (pos != _children.end() && pos->first == l)
                 ? std::pair(pos, false)
                 : std::pair(_children.emplace(pos, label(l),
                                               std::make_unique<node>()),
                             true);
    }
    value_iterator add_value(const value_type& value) {
      _values.push_front(value);
      return _values.begin();
    }

  private:
    iterator find_child_insert_pos(const label_view& l) noexcept {
      return std::lower_bound(
          _children.begin(), _children.end(), l,
          [](const auto& lhs, const auto& rhs) { return lhs.first < rhs; });
    }
    const_iterator find_child_insert_pos(const label_view& l) const noexcept {
      return std::lower_bound(
          _children.begin(), _children.end(), l,
          [](const auto& lhs, const auto& rhs) { return lhs.first < rhs; });
    }

    node_container_type _children;
    value_container_type _values;
  };

  template <typename NodePtr, typename NodeIterator, typename ValueIterator>
  class cursor_proto {
  public:
    using node_pointer = NodePtr;
    using node_iterator = NodeIterator;
    using value_iterator = ValueIterator;
    using value_reference =
        typename std::iterator_traits<value_iterator>::reference;

    bool operator==(const cursor_proto& other) const noexcept {
      return _root == other._root && _stack == other._stack &&
             _value == other._value;
    }
    bool operator!=(const cursor_proto& other) const noexcept {
      return !(*this == other);
    }

    void increment() {
      ++_value;
      move_to_next_value();
    }

    [[nodiscard]] const domain_name& domain() const noexcept { return _dname; }
    value_reference value() noexcept {
      assert(_value != current_node()->values_end() && "Bad cursor");
      return *_value;
    }

  private:
    friend class domain_tree;

    [[nodiscard]] bool is_root() const noexcept { return _stack.empty(); }
    [[nodiscard]] bool is_leaf() const noexcept {
      return current_node()->children_empty();
    }

    node_pointer current_node() const noexcept {
      return _stack.empty() ? _root : _stack.back()->second.get();
    }
    node_pointer parent_node() const noexcept {
      // clang-format off
      return _stack.empty()
          ? nullptr
          : _stack.size() == 1
              ? _root
              : _stack[_stack.size() - 2]->second.get();
      // clang-format on
    }

    void reset_value_iterator() noexcept {
      _value = current_node()->values_begin();
    }
    void decend_to_child(const node_iterator& it) {
      _stack.push_back(it);
      _dname.add_subdomain(it->first);
      reset_value_iterator();
    }
    void ascend_to_parent() noexcept {
      _stack.pop_back();
      _dname.remove_subdomain();
      reset_value_iterator();
    }

    void move_to_next_node() {
      if (node_pointer n = current_node(); !n->children_empty()) {
        decend_to_child(n->children_begin());
        return;
      }
      while (!is_root() && ++_stack.back() == parent_node()->children_end()) {
        ascend_to_parent();
      }
      if (is_root()) {
        *this = cursor_proto();
      } else {
        _dname.remove_subdomain();
        _dname.add_subdomain(_stack.back()->first);
        reset_value_iterator();
      }
    }
    void move_to_next_value() {
      for (node_pointer n = current_node(); n && _value == n->values_end();
           move_to_next_node(), n = current_node()) {}
    }

    cursor_proto() noexcept : _root(nullptr), _dname(".") {}
    explicit cursor_proto(node_pointer root)
        : _root(root), _dname("."), _value(_root->values_begin()) {}

    node_pointer _root;
    std::vector<node_iterator> _stack;
    domain_name _dname;
    value_iterator _value;
  };

public:
  using cursor = cursor_proto<node*, typename node::iterator,
                              typename node::value_iterator>;
  using const_cursor = cursor_proto<const node*, typename node::const_iterator,
                                    typename node::const_value_iterator>;

  domain_tree() : _root(std::make_unique<node>()) {}

  cursor begin() {
    cursor cur = root();
    cur.move_to_next_value();
    return cur;
  }
  const_cursor begin() const {
    const_cursor cur = root();
    cur.move_to_next_value();
    return cur;
  }

  cursor end() noexcept { return cursor(); }
  const_cursor end() const noexcept { return const_cursor(); }

  cursor insert(const domain_name& dname, const value_type& value) {
    cursor cur = insert(dname);
    cur._value = cur.current_node()->add_value(value);
    return cur;
  }

  cursor find(const domain_name& dname) {
    cursor cur = root();
    for (const auto& label : dname) {
      auto n = cur.current_node();
      if (auto child_node = n->find(label); child_node != n->children_end()) {
        cur.decend_to_child(child_node);
      } else {
        return end();
      }
    }
    return cur.current_node()->values_empty() ? end() : cur;
  }

  const_cursor find(const domain_name& dname) const {
    const_cursor cur = root();
    for (const auto& label : dname) {
      auto n = cur.current_node();
      if (auto child_node = n->find(label); child_node != n->children_end()) {
        cur.decend_to_child(child_node);
      } else {
        return end();
      }
    }
    return cur.current_node()->values_empty() ? end() : cur;
  }

  template <typename Functor>
  cursor find(const domain_name& dname, Functor f) {
    auto apply_f = [&f](cursor& cur) {
      auto n = cur.current_node();
      f(cur.domain(), n->values_begin(), n->values_end());
    };
    cursor cur = root();
    apply_f(cur);
    for (const auto& label : dname) {
      auto n = cur.current_node();
      if (auto child_node = n->find(label); child_node != n->children_end()) {
        cur.decend_to_child(child_node);
        apply_f(cur);
      } else {
        return end();
      }
    }
    return cur.current_node()->values_empty() ? end() : cur;
  }

  template <typename Functor>
  const_cursor find(const domain_name& dname, Functor f) const {
    auto apply_f = [&f](const_cursor& cur) {
      auto n = cur.current_node();
      f(cur.domain(), n->values_begin(), n->values_end());
    };
    const_cursor cur = root();
    apply_f(cur);
    for (const auto& label : dname) {
      auto n = cur.current_node();
      if (auto child_node = n->find(label); child_node != n->children_end()) {
        cur.decend_to_child(child_node);
        apply_f(cur);
      } else {
        return end();
      }
    }
    return cur.current_node()->values_empty() ? end() : cur;
  }

  friend std::ostream& operator<<(std::ostream& os, const domain_tree& dt) {
    for (auto child = dt._root->children_begin();
         child != dt._root->children_end(); ++child) {
      os << child->first << std::endl;
    }
    return os;
  }

private:
  cursor root() noexcept { return cursor(_root.get()); }
  const_cursor root() const noexcept { return const_cursor(_root.get()); }

  cursor insert(const domain_name& dname) {
    cursor cur = root();
    for (const auto& label : dname) {
      auto new_node = cur.current_node()->insert_child(label).first;
      cur.decend_to_child(new_node);
    }
    return cur;
  }

  std::unique_ptr<node> _root;
};
}  // namespace beryl
