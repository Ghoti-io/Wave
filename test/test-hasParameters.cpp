/**
 * @file
 *
 * Test the general Wave server behavior.
 */

#include <gtest/gtest.h>
#include "wave/hasParameters.hpp"

using namespace std;
using namespace Ghoti::Wave;

enum class Param {
  TEST1,
  TEST2,
};

template<>
optional<any> Ghoti::Wave::HasParameters<Param>::getParameterDefault(const Param & p) {
  if (p == Param::TEST1) {
    return uint32_t{1};
  }
  if (p == Param::TEST2) {
    return string{"foo"};
  }
  return {};
};

TEST(HasParameters, Default) {
  {
    HasParameters<Param> p{};
    ASSERT_FALSE(p.getParameter<int>(Param::TEST1));
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

