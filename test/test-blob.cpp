/**
 * @file
 *
 * Test the general Wave server behavior.
 */

#include <string>
#include <gtest/gtest.h>
#include <ghoti.io/os/file.hpp>
#include "wave/blob.hpp"

using namespace std;
using namespace Ghoti;
using namespace Ghoti::Wave;

static string tempName{"waveTest"};

TEST(Blob, General) {
  {
    // Default blob object.
    Blob b{};
    ASSERT_EQ(b.getType(), Blob::Type::TEXT);
    ASSERT_EQ(b.getText(), "");
    ASSERT_EQ(string{b.getFile()}, "");
    ASSERT_TRUE(b.sizeOrError());
    ASSERT_EQ(*b.sizeOrError(), 0);
    ASSERT_EQ(*b.lengthOrError(), 0);
  }
  {
    // Blob object using a text string.
    Blob b{"ab"};
    ASSERT_EQ(b.getType(), Blob::Type::TEXT);
    ASSERT_EQ(b.getText(), "ab");
    ASSERT_EQ(string{b.getFile()}, "");
    ASSERT_TRUE(b.sizeOrError());
    ASSERT_EQ(*b.sizeOrError(), 2);
    ASSERT_EQ(*b.lengthOrError(), 2);
  }
  {
    // Set up a temporary file and write something to it.
    auto f{OS::File::createTemp(tempName)};
    ASSERT_FALSE(f.append("ab"));

    // Verify that the file is succesfully moved into the blob object.
    Blob b{move(f)};
    ASSERT_EQ(b.getType(), Blob::Type::FILE);
    ASSERT_EQ(b.getText(), "");
    ASSERT_EQ(string{b.getFile()}, "ab");
    ASSERT_EQ(*b.sizeOrError(), 2);
    ASSERT_EQ(*b.lengthOrError(), 2);
  }
  {
    // Create a Blob with text.
    Blob b{"a"};
    EXPECT_EQ(b.getType(), Blob::Type::TEXT);
    // Convert it to a file.
    EXPECT_FALSE(b.convertToFile());
    EXPECT_EQ(b.getType(), Blob::Type::FILE);
    // Verify that the file contains the text.
    ASSERT_EQ(string{b.getFile()}, "a");
    // Write to the file and verify.
    ASSERT_FALSE(b.append("b"));
    // Verify the file wite was successful.
    ASSERT_EQ(string{b.getFile()}, "ab");
    ASSERT_EQ(*b.sizeOrError(), 2);
    ASSERT_EQ(*b.lengthOrError(), 2);
  }
  {
    // Truncating a blob with text in memory.
    Blob b{"abc"};
    ASSERT_EQ(*b.sizeOrError(), 3);
    ASSERT_FALSE(b.truncate("hello"));
    ASSERT_EQ(*b.sizeOrError(), 5);
  }
  {
    // Truncating a blob with text in a file.
    Blob b{"abc"};
    b.convertToFile();
    ASSERT_EQ(*b.sizeOrError(), 3);
    ASSERT_FALSE(b.truncate("hello"));
    ASSERT_EQ(*b.sizeOrError(), 5);
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

