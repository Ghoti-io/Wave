/**
 * @file
 *
 * Test the general Wave server behavior.
 */

#include <gtest/gtest.h>
#include "wave/hasParameters.hpp"

using namespace std;
using namespace Ghoti::Wave;

/// @cond HIDDEN_SYMBOLS
enum class Param {
  TEST1,
  TEST2,
  TEST3,
};

class HasParam : public HasParameters<Param> {
  public:
  optional<any> getParameterDefault(const Param & p) override {
    if (p == Param::TEST1) {
      return uint32_t{1};
    }
    if (p == Param::TEST2) {
      return string{"foo"};
    }
    return {};
  }
};

/// @endcond

TEST(HasParameters, Default) {
  HasParam p{};
  // Verify default values exist and are of the correct type.
  ASSERT_TRUE(p.getParameterAny(Param::TEST1));
  ASSERT_FALSE(p.getParameter<uint16_t>(Param::TEST1));
  ASSERT_TRUE(p.getParameter<uint32_t>(Param::TEST1));
  ASSERT_EQ(*p.getParameter<uint32_t>(Param::TEST1), 1);

  ASSERT_TRUE(p.getParameterAny(Param::TEST2));
  ASSERT_FALSE(p.getParameter<uint16_t>(Param::TEST2));
  ASSERT_TRUE(p.getParameter<string>(Param::TEST2));
  ASSERT_EQ(*p.getParameter<string>(Param::TEST2), "foo");

  // Verify that a default value does not exist.
  ASSERT_FALSE(p.getParameterAny(Param::TEST3));
  ASSERT_FALSE(p.getParameter<uint16_t>(Param::TEST3));
  ASSERT_FALSE(p.getParameter<string>(Param::TEST3));
}

TEST(HasParam, Set) {
  {
    HasParam p{};
    HasParam p2{};

    // Verify that `p` contains what we expect.
    ASSERT_TRUE(p.getParameterAny(Param::TEST1));
    ASSERT_EQ(*p.getParameter<uint32_t>(Param::TEST1), 1);

    // Set TEST1 to a different type.
    p.setParameter(Param::TEST1, bool{true});
    ASSERT_TRUE(p.getParameterAny(Param::TEST1));
    ASSERT_TRUE(p.getParameter<bool>(Param::TEST1));
    ASSERT_FALSE(p.getParameter<uint32_t>(Param::TEST1));
    ASSERT_EQ(*p.getParameter<bool>(Param::TEST1), true);

    // Validate that the default values are not changed for other instances.
    ASSERT_EQ(*p2.getParameter<uint32_t>(Param::TEST1), 1);
  }
  {
    // Verify that chaining works.
    HasParam p{};
    ASSERT_TRUE(p.getParameter<uint32_t>(Param::TEST1));
    ASSERT_TRUE(p.getParameter<string>(Param::TEST2));
    ASSERT_FALSE(p.getParameterAny(Param::TEST3));
    p.setParameter(Param::TEST1, double{1})
      .setParameter(Param::TEST2, double{2})
      .setParameter(Param::TEST3, double{3});
    ASSERT_TRUE(p.getParameter<double>(Param::TEST1));
    ASSERT_TRUE(p.getParameter<double>(Param::TEST2));
    ASSERT_TRUE(p.getParameter<double>(Param::TEST3));
    ASSERT_EQ(*p.getParameter<double>(Param::TEST1), 1.);
    ASSERT_EQ(*p.getParameter<double>(Param::TEST2), 2.);
    ASSERT_EQ(*p.getParameter<double>(Param::TEST3), 3.);
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

