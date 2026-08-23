// NOLINTBEGIN
#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#define ASSERT_TRUE(cond)                                                     \
  do {                                                                        \
    if (!(cond)) {                                                            \
      std::cerr << "\n  [FAIL] " << #cond << " at line " << __LINE__ << '\n'; \
      return false;                                                           \
    }                                                                         \
  } while (0)

#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))

#define RUN_TEST(test_func)                           \
  do {                                                \
    std::cout << "[RUN ] " << #test_func;             \
    if (test_func()) {                                \
      std::cout << "\n[  OK] " << #test_func << '\n'; \
      ++passed;                                       \
    } else {                                          \
      std::cout << "\n[FAIL] " << #test_func << '\n'; \
    }                                                 \
    ++total;                                          \
  } while (0)
// NOLINTEND