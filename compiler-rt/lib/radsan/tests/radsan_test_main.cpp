#include "sanitizer_test_utils.h"
// #include "gtest/gtest.h"

int main(int argc, char **argv) {

  testing::GTEST_FLAG(death_test_style) = "threadsafe";
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
