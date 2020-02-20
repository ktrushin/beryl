#include "beryl/string.hpp"

#include <gtest/gtest.h>

using beryl::str_ends_with;
using beryl::str_starts_with;

template <typename T>
class str_starts_with_test : public ::testing::Test {};

TYPED_TEST_SUITE_P(str_starts_with_test);

TYPED_TEST_P(str_starts_with_test, character) {
  EXPECT_FALSE(str_starts_with(TypeParam(""), '\0'));
  EXPECT_FALSE(str_starts_with(TypeParam(""), '0'));
  EXPECT_FALSE(str_starts_with(TypeParam(""), 'O'));
  EXPECT_FALSE(str_starts_with(TypeParam(""), 'o'));
  EXPECT_FALSE(str_starts_with(TypeParam(""), 'f'));

  EXPECT_FALSE(str_starts_with(TypeParam("a"), 'f'));
  EXPECT_TRUE(str_starts_with(TypeParam("f"), 'f'));

  EXPECT_TRUE(str_starts_with(TypeParam("fa"), 'f'));
  EXPECT_TRUE(str_starts_with(TypeParam("ffa"), 'f'));
  EXPECT_TRUE(str_starts_with(TypeParam("faf"), 'f'));
  EXPECT_FALSE(str_starts_with(TypeParam("af"), 'f'));
  EXPECT_FALSE(str_starts_with(TypeParam("aff"), 'f'));
}

TYPED_TEST_P(str_starts_with_test, empty_str_empty_prefix) {
  EXPECT_TRUE(str_starts_with(TypeParam(""), TypeParam("")));
}

TYPED_TEST_P(str_starts_with_test, empty_str) {
  EXPECT_FALSE(str_starts_with(TypeParam(""), TypeParam("f")));
  EXPECT_FALSE(str_starts_with(TypeParam(""), TypeParam("ff")));
  EXPECT_FALSE(str_starts_with(TypeParam(""), TypeParam("abc def")));
}

TYPED_TEST_P(str_starts_with_test, empty_prefix) {
  EXPECT_TRUE(str_starts_with(TypeParam("f"), TypeParam("")));
  EXPECT_TRUE(str_starts_with(TypeParam("ff"), TypeParam("")));
  EXPECT_TRUE(str_starts_with(TypeParam("abc def"), TypeParam("")));
}

TYPED_TEST_P(str_starts_with_test, one_char_str) {
  EXPECT_FALSE(str_starts_with(TypeParam("f"), TypeParam("o")));
  EXPECT_TRUE(str_starts_with(TypeParam("f"), TypeParam("f")));
  EXPECT_FALSE(str_starts_with(TypeParam("f"), TypeParam("ff")));
  EXPECT_FALSE(str_starts_with(TypeParam("f"), TypeParam("oo")));
  EXPECT_FALSE(str_starts_with(TypeParam("f"), TypeParam("bar")));
}

TYPED_TEST_P(str_starts_with_test, one_char_prefix) {
  // clang-format off
  EXPECT_FALSE(str_starts_with(TypeParam(  "f"), TypeParam("o")));
  EXPECT_FALSE(str_starts_with(TypeParam( "ff"), TypeParam("o")));
  EXPECT_FALSE(str_starts_with(TypeParam( "fg"), TypeParam("o")));
  EXPECT_TRUE (str_starts_with(TypeParam( "oo"), TypeParam("o")));
  EXPECT_TRUE (str_starts_with(TypeParam("ooo"), TypeParam("o")));
  EXPECT_TRUE (str_starts_with(TypeParam("oao"), TypeParam("o")));
  EXPECT_FALSE(str_starts_with(TypeParam("aoo"), TypeParam("o")));
  // clang-format on
}

TYPED_TEST_P(str_starts_with_test, starts_with_prefix) {
  EXPECT_TRUE(str_starts_with(TypeParam("abcwxyz"), TypeParam("abc")));
  EXPECT_TRUE(str_starts_with(TypeParam("abcwxyzabc"), TypeParam("abc")));
  EXPECT_TRUE(str_starts_with(TypeParam("abcwxyzabcyz"), TypeParam("abc")));
}

TYPED_TEST_P(str_starts_with_test, does_not_start_with_prefix) {
  EXPECT_FALSE(str_starts_with(TypeParam("wxyz"), TypeParam("abc")));
  EXPECT_FALSE(str_starts_with(TypeParam("wxyzabc"), TypeParam("abc")));
  EXPECT_FALSE(str_starts_with(TypeParam("yzabcwxyzyzabc"), TypeParam("abc")));
}

TYPED_TEST_P(str_starts_with_test, str_is_substr_of_prefix) {
  EXPECT_FALSE(str_starts_with(TypeParam("ab"), TypeParam("abcxyz")));
  EXPECT_FALSE(str_starts_with(TypeParam("abcxy"), TypeParam("abcxyz")));
  EXPECT_FALSE(str_starts_with(TypeParam("z"), TypeParam("abcxyz")));
  EXPECT_FALSE(str_starts_with(TypeParam("bcxyz"), TypeParam("abcxyz")));
  EXPECT_FALSE(str_starts_with(TypeParam("cx"), TypeParam("abcxyz")));
  EXPECT_FALSE(str_starts_with(TypeParam("bcxy"), TypeParam("abcxyz")));
}

TYPED_TEST_P(str_starts_with_test, full_match) {
  EXPECT_TRUE(str_starts_with(TypeParam("abc"), TypeParam("abc")));
  EXPECT_TRUE(str_starts_with(TypeParam("ffff"), TypeParam("ffff")));
  EXPECT_TRUE(str_starts_with(TypeParam("1)2(3*4 5^7%$9#0@=!"),
                              TypeParam("1)2(3*4 5^7%$9#0@=!")));
}

REGISTER_TYPED_TEST_SUITE_P(str_starts_with_test, character,
                            empty_str_empty_prefix, empty_str, empty_prefix,
                            one_char_str, one_char_prefix, starts_with_prefix,
                            does_not_start_with_prefix, str_is_substr_of_prefix,
                            full_match);
using str_starts_with_test_types =
    ::testing::Types<std::string_view, std::string, boost::container::string>;
INSTANTIATE_TYPED_TEST_SUITE_P(_, str_starts_with_test,
                               str_starts_with_test_types, );

template <typename T>
class str_ends_with_test : public ::testing::Test {};

TYPED_TEST_SUITE_P(str_ends_with_test);

TYPED_TEST_P(str_ends_with_test, character) {
  EXPECT_FALSE(str_ends_with(TypeParam(""), '\0'));
  EXPECT_FALSE(str_ends_with(TypeParam(""), '0'));
  EXPECT_FALSE(str_ends_with(TypeParam(""), 'O'));
  EXPECT_FALSE(str_ends_with(TypeParam(""), 'o'));
  EXPECT_FALSE(str_ends_with(TypeParam(""), 'f'));

  EXPECT_FALSE(str_ends_with(TypeParam("a"), 'f'));
  EXPECT_TRUE(str_ends_with(TypeParam("f"), 'f'));

  EXPECT_TRUE(str_ends_with(TypeParam("af"), 'f'));
  EXPECT_TRUE(str_ends_with(TypeParam("aff"), 'f'));
  EXPECT_TRUE(str_ends_with(TypeParam("faf"), 'f'));
  EXPECT_FALSE(str_ends_with(TypeParam("fa"), 'f'));
  EXPECT_FALSE(str_ends_with(TypeParam("afa"), 'f'));
}

TYPED_TEST_P(str_ends_with_test, empty_str_empty_suffix) {
  EXPECT_TRUE(str_ends_with(TypeParam(""), TypeParam("")));
}

TYPED_TEST_P(str_ends_with_test, empty_str) {
  EXPECT_FALSE(str_ends_with(TypeParam(""), TypeParam("f")));
  EXPECT_FALSE(str_ends_with(TypeParam(""), TypeParam("ff")));
  EXPECT_FALSE(str_ends_with(TypeParam(""), TypeParam("abc def")));
}

TYPED_TEST_P(str_ends_with_test, empty_suffix) {
  EXPECT_TRUE(str_ends_with(TypeParam("f"), TypeParam("")));
  EXPECT_TRUE(str_ends_with(TypeParam("ff"), TypeParam("")));
  EXPECT_TRUE(str_ends_with(TypeParam("abc def"), TypeParam("")));
}

TYPED_TEST_P(str_ends_with_test, one_char_str) {
  EXPECT_FALSE(str_ends_with(TypeParam("f"), TypeParam("o")));
  EXPECT_TRUE(str_ends_with(TypeParam("f"), TypeParam("f")));
  EXPECT_FALSE(str_ends_with(TypeParam("f"), TypeParam("ff")));
  EXPECT_FALSE(str_ends_with(TypeParam("f"), TypeParam("oo")));
  EXPECT_FALSE(str_ends_with(TypeParam("f"), TypeParam("bar")));
}

TYPED_TEST_P(str_ends_with_test, one_char_suffix) {
  // clang-format off
  EXPECT_FALSE(str_ends_with(TypeParam(  "f"), TypeParam("o")));
  EXPECT_FALSE(str_ends_with(TypeParam( "ff"), TypeParam("o")));
  EXPECT_FALSE(str_ends_with(TypeParam( "fg"), TypeParam("o")));
  EXPECT_TRUE (str_ends_with(TypeParam( "oo"), TypeParam("o")));
  EXPECT_TRUE (str_ends_with(TypeParam("ooo"), TypeParam("o")));
  EXPECT_TRUE (str_ends_with(TypeParam("oao"), TypeParam("o")));
  EXPECT_FALSE(str_ends_with(TypeParam("ooa"), TypeParam("o")));
  // clang-format on
}

TYPED_TEST_P(str_ends_with_test, ends_with_suffix) {
  EXPECT_TRUE(str_ends_with(TypeParam("abcdxyz"), TypeParam("xyz")));
  EXPECT_TRUE(str_ends_with(TypeParam("xyzabcdxyz"), TypeParam("xyz")));
  EXPECT_TRUE(str_ends_with(TypeParam("efxyzabcdxyz"), TypeParam("xyz")));
}

TYPED_TEST_P(str_ends_with_test, does_not_end_with_suffix) {
  EXPECT_FALSE(str_ends_with(TypeParam("abcd"), TypeParam("xyz")));
  EXPECT_FALSE(str_ends_with(TypeParam("xyzabcd"), TypeParam("xyz")));
  EXPECT_FALSE(str_ends_with(TypeParam("efxyzabcd"), TypeParam("xyz")));
}

TYPED_TEST_P(str_ends_with_test, str_is_substr_of_suffix) {
  EXPECT_FALSE(str_ends_with(TypeParam("ab"), TypeParam("abcxyz")));
  EXPECT_FALSE(str_ends_with(TypeParam("abcxy"), TypeParam("abcxyz")));
  EXPECT_FALSE(str_ends_with(TypeParam("z"), TypeParam("abcxyz")));
  EXPECT_FALSE(str_ends_with(TypeParam("bcxyz"), TypeParam("abcxyz")));
  EXPECT_FALSE(str_ends_with(TypeParam("cx"), TypeParam("abcxyz")));
  EXPECT_FALSE(str_ends_with(TypeParam("bcxy"), TypeParam("abcxyz")));
}

TYPED_TEST_P(str_ends_with_test, full_match) {
  EXPECT_TRUE(str_ends_with(TypeParam("abc"), TypeParam("abc")));
  EXPECT_TRUE(str_ends_with(TypeParam("ffff"), TypeParam("ffff")));
  EXPECT_TRUE(str_ends_with(TypeParam("1)2(3*4 5^7%8$9#0@=!"),
                            TypeParam("1)2(3*4 5^7%8$9#0@=!")));
}

REGISTER_TYPED_TEST_SUITE_P(str_ends_with_test, character,
                            empty_str_empty_suffix, empty_str, empty_suffix,
                            one_char_str, one_char_suffix, ends_with_suffix,
                            does_not_end_with_suffix, str_is_substr_of_suffix,
                            full_match);
using str_ends_with_test_types =
    ::testing::Types<std::string_view, std::string, boost::container::string>;
INSTANTIATE_TYPED_TEST_SUITE_P(_, str_ends_with_test,
                               str_ends_with_test_types, );
