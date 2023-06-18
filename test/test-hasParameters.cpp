/**
 * @file
 *
 * Test the general Wave server behavior.
 */

#include <gtest/gtest.h>
#include "wave/hasParameters.hpp"

using namespace std;
using namespace Ghoti::Wave;

TEST(HasParameters, Default) {
  {
    HasParameters p{};
    ASSERT_FALSE(p.getParameter<int>(Parameter::MAXBUFFERSIZE));
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

