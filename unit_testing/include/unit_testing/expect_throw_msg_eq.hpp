#pragma once

#define EXPECT_THROW_MSG_EQ(statement, exception_type, msg) \
  try {                                                     \
    (statement);                                            \
    FAIL() << "expected to throw but it didn't";            \
  } catch (const exception_type& e) {                       \
    EXPECT_STREQ(e.what(), msg);                            \
  } catch (...) {                                           \
    FAIL() << "expected to throw " << #exception_type       \
           << " but it threw something else";               \
  }
