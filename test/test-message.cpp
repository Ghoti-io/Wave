/**
 * @file
 *
 * Test the general Wave server behavior.
 */

#include <string>
#include <gtest/gtest.h>
#include <ghoti.io/util/file.hpp>
#include "wave.hpp"

using namespace std;
using namespace Ghoti;
using namespace Ghoti::Wave;


TEST(Message, Defaults) {
  {
    Message m{Message::Type::REQUEST};
    ASSERT_EQ(m.getType(), Message::Type::REQUEST);
    ASSERT_EQ(m.getTransport(), Message::Transport::UNDECLARED);
    ASSERT_EQ(m.getTarget(), "");
    ASSERT_EQ(m.getDomain(), "");
    ASSERT_EQ(m.getPort(), 0);
    ASSERT_FALSE(m.hasError());
    ASSERT_EQ(m.getStatusCode(), 0);
    ASSERT_EQ(m.getMessage(), "");
    ASSERT_EQ(m.getMethod(), "GET");
    ASSERT_EQ(m.getVersion(), "");
    ASSERT_EQ(m.getTarget(), "");
    ASSERT_EQ(m.getMessageBody(), "");
    ASSERT_EQ(m.getFields().size(), 0);
    ASSERT_EQ(m.getContentLength(), 0);
    ASSERT_EQ(m.getId(), 0);
  }
  {
    Message m{Message::Type::RESPONSE};
    ASSERT_EQ(m.getType(), Message::Type::RESPONSE);
    ASSERT_EQ(m.getTransport(), Message::Transport::UNDECLARED);
    ASSERT_EQ(m.getTarget(), "");
    ASSERT_EQ(m.getDomain(), "");
    ASSERT_EQ(m.getPort(), 0);
    ASSERT_FALSE(m.hasError());
    ASSERT_EQ(m.getStatusCode(), 0);
    ASSERT_EQ(m.getMessage(), "");
    ASSERT_EQ(m.getMethod(), "GET");
    ASSERT_EQ(m.getVersion(), "");
    ASSERT_EQ(m.getTarget(), "");
    ASSERT_EQ(m.getMessageBody(), "");
    ASSERT_EQ(m.getFields().size(), 0);
    ASSERT_EQ(m.getContentLength(), 0);
    ASSERT_EQ(m.getId(), 0);
  }
}

TEST(Message, Fields) {
  {
    Message m{Message::Type::REQUEST};
    m.setDomain("127.0.0.1")
      .setPort(80)
      .setTarget("/foo")
      .addFieldValue("x1", "a")
      .addFieldValue("x1", "b")
      .addFieldValue("x1", "b")
      .addFieldValue("x2", "a")
      .addFieldValue("x3", "c")
      .addFieldValue("x4", "c\"");

    // Verify that the expected number of fields are present.
    auto fields = m.getFields();
    ASSERT_EQ(fields.size(), 4);

    auto x1 = fields.at("x1");
    auto x2 = fields.at("x2");
    auto x3 = fields.at("x3");
    auto x4 = fields.at("x4");
    ASSERT_EQ(x1.size(), 3);
    ASSERT_EQ(x2.size(), 1);
    ASSERT_EQ(x3.size(), 1);
    ASSERT_EQ(x4.size(), 1);

    // Verify that a single field can have multiple values, even repeated.
    ASSERT_EQ(x1[0], "a");
    ASSERT_EQ(x1[1], "b");
    ASSERT_EQ(x1[2], "b");

    // Verify that different fields can have the same value.
    ASSERT_EQ(x2[0], "a");

    // Verify that different fields can have a unique value.
    ASSERT_EQ(x3[0], "c");

    // Verify that double-quoted values have the escaped characters properly
    // interpreted.
    ASSERT_EQ(x4[0], "c\"");
  }
}

TEST(Message, Chunks) {
  {
    Message m{Message::Type::REQUEST};
    ASSERT_EQ(m.getTransport(), Message::Transport::UNDECLARED);
    ASSERT_EQ(m.getChunks().size(), 0);
    Blob b{};
    b.append("hello");
    m.addChunk(move(b));
    ASSERT_EQ(m.getTransport(), Message::Transport::CHUNKED);
    ASSERT_EQ(m.getChunks().size(), 1);
    b = {};
    b.append("world!");
    m.addChunk(move(b));
    ASSERT_EQ(m.getTransport(), Message::Transport::CHUNKED);
    ASSERT_EQ(m.getChunks().size(), 2);
    cout << "CHUNKED" << endl;
    cout << m << endl;
    cout << "--------------------------" << endl;
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

