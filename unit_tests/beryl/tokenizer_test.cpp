#include "beryl/tokenizer.hpp"

#include <gtest/gtest.h>

namespace {
void expect_tokens_eq(const char* str, char delimiter,
                      const std::vector<std::string_view>& expected_tokens) {
  std::ostringstream trace_msg;
  trace_msg << "tokenizing `" << str << "` with `" << delimiter << "`";
  SCOPED_TRACE(trace_msg.str());

  std::vector<std::string_view> tokens;
  beryl::tokenizer t(str, delimiter);
  for (const auto& token : t) {
    tokens.push_back(token);
  }
  EXPECT_EQ(tokens, expected_tokens);
}
}  // namespace

TEST(tokenizer_test, yields_expected_result) {
  expect_tokens_eq("foo.bar.baz", '.', {"foo", "bar", "baz"});

  expect_tokens_eq("", '*', {""});
  expect_tokens_eq("f", 'f', {"", ""});
  expect_tokens_eq("####", '#', {"", "", "", "", ""});

  expect_tokens_eq("a", '-', {"a"});
  expect_tokens_eq("a-", '-', {"a", ""});
  expect_tokens_eq("a--", '-', {"a", "", ""});
  expect_tokens_eq("-a", '-', {"", "a"});
  expect_tokens_eq("--a", '-', {"", "", "a"});
  expect_tokens_eq("-a-", '-', {"", "a", ""});
  expect_tokens_eq("--a--", '-', {"", "", "a", "", ""});
  expect_tokens_eq("-a-b-", '-', {"", "a", "b", ""});
  expect_tokens_eq("-a---b-", '-', {"", "a", "", "", "b", ""});

  expect_tokens_eq("kilo", '^', {"kilo"});
  expect_tokens_eq("kilo^", '^', {"kilo", ""});
  expect_tokens_eq("kilo^^", '^', {"kilo", "", ""});
  expect_tokens_eq("^kilo", '^', {"", "kilo"});
  expect_tokens_eq("^^kilo", '^', {"", "", "kilo"});
  expect_tokens_eq("^kilo^", '^', {"", "kilo", ""});
  expect_tokens_eq("^^kilo^^", '^', {"", "", "kilo", "", ""});
  expect_tokens_eq("^kilo^lima^", '^', {"", "kilo", "lima", ""});
  expect_tokens_eq("^kilo^^^lima^", '^', {"", "kilo", "", "", "lima", ""});
}
