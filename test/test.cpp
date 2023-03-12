/**
 * @file
 *
 * Test the general Wave server behavior.
 */

#include<gtest/gtest.h>
#include "wave.hpp"

using namespace std;
using namespace Ghoti::Wave;

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

