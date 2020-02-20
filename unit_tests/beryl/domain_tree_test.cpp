#include "beryl/domain_tree.hpp"

#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "unit_testing/expect_throw_msg_eq.hpp"

using domain_name = beryl::domain_name;
template <typename T>
using domain_tree = beryl::domain_tree<T>;
using domain_name = beryl::domain_name;

TEST(domain_tree_test, empty) {
  domain_tree<int> t;
  EXPECT_EQ(t.begin(), t.end());
  EXPECT_EQ(t.find(domain_name("alpha.")), t.end());
  EXPECT_EQ(t.find(domain_name("bravo.alpha.")), t.end());
  EXPECT_EQ(t.find(domain_name("charlie.bravo.alpha.")), t.end());
  EXPECT_EQ(t.find(domain_name("delta.charlie.bravo.alpha.")), t.end());
}

namespace {
void test_insertion(const std::string& dname,
                    const std::array<int, 3>& values) {
  SCOPED_TRACE(dname);

  domain_tree<int> t;

  auto cur = t.insert(domain_name(dname), values[0]);
  ASSERT_NE(cur, t.end());
  EXPECT_EQ(cur.domain(), domain_name(dname));
  ASSERT_EQ(cur.value(), values[0]);

  cur = t.insert(domain_name(dname), values[1]);
  ASSERT_NE(cur, t.end());
  EXPECT_EQ(cur.domain(), domain_name(dname));
  ASSERT_EQ(cur.value(), values[1]);
}

domain_tree<int> generate_domain_tree(
    std::vector<std::pair<std::string, std::vector<int>>> key_vals_pairs) {
  domain_tree<int> dtree;
  for (const auto& [k, vals] : key_vals_pairs) {
    for (const auto& val : vals) {
      dtree.insert(domain_name(k), val);
    }
  }
  return dtree;
}

void expect_domain_tree_eq(
    const std::string& msg, const domain_tree<int>& dtree,
    const std::vector<std::pair<std::string, std::multiset<int>>>& expected) {
  SCOPED_TRACE(msg);
  std::vector<std::pair<std::string, std::multiset<int>>> got;
  for (auto cur = dtree.begin(); cur != dtree.end(); cur.increment()) {
    auto dname = to_string(cur.domain());
    if (got.empty() || got.back().first != dname) {
      got.push_back(std::make_pair(dname, std::multiset<int>()));
    }
    got.back().second.insert(cur.value());
  }
  EXPECT_EQ(got, expected);
}
}  // namespace

TEST(domain_tree_test, insertion) {
  test_insertion(".", {0, 0});
  test_insertion(".", {1, 2});
  test_insertion("alpha.", {1, 2});
  test_insertion("bravo.alpha.", {1, 2});
  test_insertion("charlie.bravo.alpha.", {1, 2});
  test_insertion("delta.charlie.bravo.alpha.", {1, 2});
}

// clang-format off
TEST(domain_tree_test, iteration) {
  {
    auto dtree = generate_domain_tree({{".", {33, 22}}});
    expect_domain_tree_eq("root domain only", dtree, {{".", {22, 33}}});
  }
  {
    auto dtree = generate_domain_tree({
      {"delta.", {7, 11}},
      {"bravo.", {2, 7}},
      {"alpha.", {1}},
      {"charlie.", {6, 4}},
      {"echo.", {19, 5}}
    });
    expect_domain_tree_eq("ordering top level domains", dtree, {
      {".alpha", {1}},
      {".bravo", {2, 7}},
      {".charlie", {4, 6}},
      {".delta", {7, 11}},
      {".echo", {5, 19}}
    });
  }
  {
    auto dtree = generate_domain_tree({
      {".", {1}},
      {"delta.alpha.", {4, 44}},
      {"echo.alpha.", {5, 55}},
      {"bravo.alpha.", {2}},
      {"charlie.alpha.", {33, 3}},
      {"foxrot.alpha.", {7}}
    });
    expect_domain_tree_eq("ordering second level domains", dtree, {
      {".", {1}},
      {".alpha.bravo", {2}},
      {".alpha.charlie", {3, 33}},
      {".alpha.delta", {44, 4}},
      {".alpha.echo", {55, 5}},
      {".alpha.foxrot", {7}}
    });
  }
  {
    auto dtree = generate_domain_tree({
      {"juliett.india.golf.", {6, 66}},
      {"delta.alpha.", {3}},
      {"foxtrot.echo.", {44, 4, 444}},
      {"hotel.golf.", {555, 55, 5}},
      {"charlie.bravo.alpha.", {22, 2}},
      {"mike.lima.kilo.india.golf.", {77, 7}}
    });
    expect_domain_tree_eq("ordering random domain mix", dtree, {
      {".alpha.bravo.charlie", {2, 22}},
      {".alpha.delta", {3}},
      {".echo.foxtrot", {4, 44, 444}},
      {".golf.hotel", {5, 55, 555}},
      {".golf.india.juliett", {6, 66}},
      {".golf.india.kilo.lima.mike", {7, 77}}
    });
  }
}
// clang-format on

TEST(domain_tree_test, find) {
  auto dtree = generate_domain_tree({{"charlie.bravo.alpha.", {22, 2}},
                                     {"delta.alpha.", {3}},
                                     {"foxtrot.echo.", {44, 4, 444}},
                                     {"hotel.golf.", {555, 55, 5}},
                                     {"juliett.india.golf.", {6, 66}},
                                     {"mike.lima.kilo.india.golf.", {77, 7}}});
  EXPECT_EQ(dtree.find(domain_name(".")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("alpha.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("xray.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("bravo.alpha.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("echo.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("hotel.echo.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("golf.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("india.golf.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("kilo.india.golf.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("mike.lima.kilo.india.xray.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("mike.lima.kilo.xray.golf.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("mike.lima.xray.india.golf.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("mike.xray.kilo.india.golf.")), dtree.end());
  EXPECT_EQ(dtree.find(domain_name("xray.lima.kilo.india.golf.")), dtree.end());
  {
    auto cur = dtree.find(domain_name("charlie.bravo.alpha."));
    EXPECT_NE(cur, dtree.end());
    EXPECT_EQ(cur.domain(), domain_name("charlie.bravo.alpha."));
    EXPECT_TRUE(cur.value() == 2 || cur.value() == 22);
  }
  {
    auto cur = dtree.find(domain_name("delta.alpha."));
    EXPECT_NE(cur, dtree.end());
    EXPECT_EQ(cur.domain(), domain_name("delta.alpha."));
    EXPECT_EQ(cur.value(), 3);
  }
  {
    auto cur = dtree.find(domain_name("foxtrot.echo."));
    EXPECT_NE(cur, dtree.end());
    EXPECT_EQ(cur.domain(), domain_name("foxtrot.echo."));
    EXPECT_TRUE(cur.value() == 4 || cur.value() == 44 || cur.value() == 444);
  }
  {
    auto cur = dtree.find(domain_name("hotel.golf."));
    EXPECT_NE(cur, dtree.end());
    EXPECT_EQ(cur.domain(), domain_name("hotel.golf."));
    EXPECT_TRUE(cur.value() == 5 || cur.value() == 55 || cur.value() == 555);
  }
  {
    auto cur = dtree.find(domain_name("juliett.india.golf."));
    EXPECT_NE(cur, dtree.end());
    EXPECT_EQ(cur.domain(), domain_name("juliett.india.golf."));
    EXPECT_TRUE(cur.value() == 6 || cur.value() == 66);
  }
  {
    auto cur = dtree.find(domain_name("mike.lima.kilo.india.golf."));
    EXPECT_NE(cur, dtree.end());
    EXPECT_EQ(cur.domain(), domain_name("mike.lima.kilo.india.golf."));
    EXPECT_TRUE(cur.value() == 7 || cur.value() == 77);
  }
}
