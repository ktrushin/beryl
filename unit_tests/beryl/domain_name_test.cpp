#include "beryl/domain_name.hpp"

#include <initializer_list>

#include <gtest/gtest.h>

#include "unit_testing/expect_throw_msg_eq.hpp"

using domain_name = beryl::domain_name;
using label_view = beryl::label_view;

namespace {
void expect_invalid_domain_name(const std::string& fully_qualified_domain_name) {
  SCOPED_TRACE(fully_qualified_domain_name);
  std::ostringstream msg;
  msg << "invalid domain name: `" << fully_qualified_domain_name << "`";
  EXPECT_THROW_MSG_EQ(domain_name(fully_qualified_domain_name),
                      std::runtime_error, msg.str().c_str());
}

void expect_domain_name_eq(
    const std::string& fully_qualified_domain_name,
    std::initializer_list<const char*> raw_expected_labels) {
  SCOPED_TRACE(fully_qualified_domain_name);
  std::vector<label_view> labels;
  for (const auto& label : domain_name(fully_qualified_domain_name)) {
    labels.push_back(label);
  }
  std::vector<label_view> expected_labels;
  for (const auto& raw_label : raw_expected_labels) {
    expected_labels.emplace_back(raw_label);
  }
  EXPECT_EQ(labels, expected_labels);
}
}  // namespace

TEST(domain_name_str_ctor_test, root_domain) { expect_domain_name_eq(".", {}); }
TEST(domain_name_str_ctor_test, top_level_domain) {
  expect_domain_name_eq("a.", {"a"});
  expect_domain_name_eq("ab.", {"ab"});
  expect_domain_name_eq("a1.", {"a1"});
  expect_domain_name_eq("a1b.", {"a1b"});
  expect_domain_name_eq("a-1-b.", {"a-1-b"});
  expect_domain_name_eq("1.", {"1"});
  expect_domain_name_eq("1a.", {"1a"});
  expect_domain_name_eq("12.", {"12"});
  expect_domain_name_eq("1a2.", {"1a2"});
  expect_domain_name_eq("1-a-2.", {"1-a-2"});
  expect_domain_name_eq("alpha-3.", {"alpha-3"});
}

TEST(domain_name_str_ctor_test, second_level_domain) {
  expect_domain_name_eq("a.mike.", {"mike", "a"});
  expect_domain_name_eq("ab.mike.", {"mike", "ab"});
  expect_domain_name_eq("a1.mike.", {"mike", "a1"});
  expect_domain_name_eq("a1b.mike.", {"mike", "a1b"});
  expect_domain_name_eq("a-1-b.mike.", {"mike", "a-1-b"});
  expect_domain_name_eq("1.mike.", {"mike", "1"});
  expect_domain_name_eq("1a.mike.", {"mike", "1a"});
  expect_domain_name_eq("12.mike.", {"mike", "12"});
  expect_domain_name_eq("1a2.mike.", {"mike", "1a2"});
  expect_domain_name_eq("1-a-2.mike.", {"mike", "1-a-2"});
  expect_domain_name_eq("alpha-3.mike.", {"mike", "alpha-3"});
}

TEST(domain_name_str_ctor_test, third_level_domain) {
  expect_domain_name_eq("a.lima.mike.", {"mike", "lima", "a"});
  expect_domain_name_eq("ab.lima.mike.", {"mike", "lima", "ab"});
  expect_domain_name_eq("a1.lima.mike.", {"mike", "lima", "a1"});
  expect_domain_name_eq("a1b.lima.mike.", {"mike", "lima", "a1b"});
  expect_domain_name_eq("a-1-b.lima.mike.", {"mike", "lima", "a-1-b"});
  expect_domain_name_eq("1.lima.mike.", {"mike", "lima", "1"});
  expect_domain_name_eq("1a.lima.mike.", {"mike", "lima", "1a"});
  expect_domain_name_eq("12.lima.mike.", {"mike", "lima", "12"});
  expect_domain_name_eq("1a2.lima.mike.", {"mike", "lima", "1a2"});
  expect_domain_name_eq("1-a-2.lima.mike.", {"mike", "lima", "1-a-2"});
  expect_domain_name_eq("alpha-3.lima.mike.", {"mike", "lima", "alpha-3"});
}

TEST(domain_name_str_ctor_test, fourth_level_domain) {
  expect_domain_name_eq("a.kilo.lima.mike.", {"mike", "lima", "kilo", "a"});
  expect_domain_name_eq("ab.kilo.lima.mike.", {"mike", "lima", "kilo", "ab"});
  expect_domain_name_eq("a1.kilo.lima.mike.", {"mike", "lima", "kilo", "a1"});
  expect_domain_name_eq("a1b.kilo.lima.mike.", {"mike", "lima", "kilo", "a1b"});
  expect_domain_name_eq("a-1-b.kilo.lima.mike.",
                        {"mike", "lima", "kilo", "a-1-b"});
  expect_domain_name_eq("1.kilo.lima.mike.", {"mike", "lima", "kilo", "1"});
  expect_domain_name_eq("1a.kilo.lima.mike.", {"mike", "lima", "kilo", "1a"});
  expect_domain_name_eq("12.kilo.lima.mike.", {"mike", "lima", "kilo", "12"});
  expect_domain_name_eq("1a2.kilo.lima.mike.", {"mike", "lima", "kilo", "1a2"});
  expect_domain_name_eq("1-a-2.kilo.lima.mike.",
                        {"mike", "lima", "kilo", "1-a-2"});
  expect_domain_name_eq("alpha-3.kilo.lima.mike.",
                        {"mike", "lima", "kilo", "alpha-3"});
}

TEST(domain_name_str_ctor_test, lowers_chars) {
  expect_domain_name_eq("kIlO1-LiMa2.MIKE.", {"mike", "kilo1-lima2"});
}

TEST(domain_name_str_ctor_test, empty_str_is_not_ok) {
  expect_invalid_domain_name("");
}

TEST(domain_name_str_ctor_test, misssing_final_dot_is_not_ok) {
  expect_invalid_domain_name("a");
  expect_invalid_domain_name("ab.mike");
  expect_invalid_domain_name("abc.lima.mike");
  expect_invalid_domain_name("abcd.kilo.lima.mike");
}

TEST(domain_name_str_ctor_test, too_long_string_is_not_ok) {
  std::string str;
  for (std::size_t i = 0; i < 50; ++i) {
    str.append("yyyy.");
  }
  std::string valid = str + "abcd.";
  ASSERT_EQ(valid.size(), 255);
  EXPECT_NO_THROW((domain_name(valid)));

  std::string invalid = str + "abcde.";
  ASSERT_EQ(invalid.size(), 256);
  expect_invalid_domain_name(invalid);
}

TEST(domain_name_str_ctor_test, too_many_labels_is_not_ok) {
  auto gen_str = [](std::size_t label_count) {
    std::string str;
    // @note This is an example of a label of the minimal possible length.
    std::string label = "r.";
    str.reserve(label_count * label.size());
    for (std::size_t i = 0; i < label_count; ++i) {
      str.append(label);
    }
    return str;
  };
  EXPECT_NO_THROW((domain_name(gen_str(127))));
  expect_invalid_domain_name(gen_str(128));
}

TEST(domain_name_str_ctor_test, empty_label_is_not_ok) {
  expect_invalid_domain_name("ab..");
  expect_invalid_domain_name(".ab.");
  expect_invalid_domain_name("ab..mike.");
  expect_invalid_domain_name("..ab.mike.");
}

TEST(domain_name_str_ctor_test, too_long_label_is_not_ok) {
  EXPECT_NO_THROW((domain_name(std::string(63, 'a') + ".")));
  expect_invalid_domain_name(std::string(64, 'a') + ".");
  EXPECT_NO_THROW((domain_name(std::string(63, 'f') + ".kilo." +
                               std::string(63, 'a') + ".mike.")));
  expect_invalid_domain_name(std::string(64, 'f') + ".kilo." +
                             std::string(64, 'a') + ".mike.");
}

TEST(domain_name_str_ctor_test,
     label_first_char_is_not_letter_or_digit_is_not_ok) {
  EXPECT_NO_THROW((domain_name("kilo.lima.mike.")));
  EXPECT_NO_THROW((domain_name("kilo.Lima.mike.")));
  EXPECT_NO_THROW((domain_name("kilo.1ima.mike.")));
  expect_invalid_domain_name("kilo.-ima.mike.");
  expect_invalid_domain_name("kilo.*ima.mike.");
}

TEST(domain_name_str_ctor_test,
     label_last_char_is_not_letter_or_digit_is_not_ok) {
  EXPECT_NO_THROW((domain_name("kilo.lima.mike.")));
  EXPECT_NO_THROW((domain_name("kilo.limA.mike.")));
  EXPECT_NO_THROW((domain_name("kilo.lim3.mike.")));
  expect_invalid_domain_name("kilo.lim-.mike.");
  expect_invalid_domain_name("kilo.lim*.mike.");
}

TEST(domain_name_str_ctor_test,
     label_interior_char_is_not_letter_digit_or_hiphen_is_not_ok) {
  EXPECT_NO_THROW((domain_name("kilo.lima.mike.")));
  EXPECT_NO_THROW((domain_name("kilo.liMa.mike.")));
  EXPECT_NO_THROW((domain_name("kilo.li3a.mike.")));
  EXPECT_NO_THROW((domain_name("kilo.li-a.mike.")));
  expect_invalid_domain_name("kilo.li*a.mike.");
  expect_invalid_domain_name("kilo.li@a.mike.");
}

TEST(domain_name_add_subdomain_test, yields_expected_result) {
  // clang-format off
  EXPECT_EQ(domain_name(".").add_subdomain(label_view("foo")),
            domain_name("foo."));
  // clang-format on
  EXPECT_EQ(domain_name("alpha.").add_subdomain(label_view("bravo")),
            domain_name("bravo.alpha."));
  EXPECT_EQ(domain_name("bravo.alpha.").add_subdomain(label_view("charlie")),
            domain_name("charlie.bravo.alpha."));
}

TEST(domain_name_remove_subdomain_test, yields_expected_result) {
  EXPECT_EQ(domain_name("alpha.").remove_subdomain(), domain_name("."));
  EXPECT_EQ(domain_name("bravo.alpha.").remove_subdomain(), domain_name("alpha"
                                                                        "."));
  EXPECT_EQ(domain_name("charlie.bravo.alpha.").remove_subdomain(),
            domain_name("bravo.alpha."));
}

TEST(domain_name_equals_test, equal) {
  EXPECT_EQ(domain_name("."), domain_name("."));
  EXPECT_EQ(domain_name("alpha."), domain_name("alpha."));
  EXPECT_EQ(domain_name("bravo.alpha."), domain_name("bravo.alpha."));
}

TEST(domain_name_equals_test, not_equal) {
  EXPECT_NE(domain_name("."), domain_name("alpha."));
  EXPECT_NE(domain_name("alpha."), domain_name("."));
  EXPECT_NE(domain_name("bravo.alpha."), domain_name("alpha."));
  EXPECT_NE(domain_name("alpha."), domain_name("bravo.alpha."));
}
