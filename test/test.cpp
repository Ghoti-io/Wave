/**
 * @file
 *
 * Test the general Wave server behavior.
 */

#include <string>
#include <gtest/gtest.h>
#include <ghoti.io/os/file.hpp>
#include "wave.hpp"

using namespace std;
using namespace Ghoti;
using namespace Ghoti::Wave;

static Server s{};
static uint16_t serverPort = 0;
static string tempName{"waveTest"};

constexpr auto quantum{10ms};

TEST(Blob, General) {
  {
    // Default blob object.
    Blob b{};
    ASSERT_EQ(b.getType(), Blob::Type::TEXT);
    ASSERT_EQ(b.getText(), "");
    ASSERT_EQ(string{b.getFile()}, "");
    ASSERT_EQ(b.size(), 0);
    ASSERT_EQ(b.length(), 0);
  }
  {
    // Blob object using a text string.
    Blob b{"a"};
    ASSERT_EQ(b.getType(), Blob::Type::TEXT);
    ASSERT_EQ(b.getText(), "a");
    ASSERT_EQ(string{b.getFile()}, "");
    ASSERT_EQ(b.size(), 1);
    ASSERT_EQ(b.length(), 1);
  }
  {
    // Set up a temporary file and write something to it.
    auto f{OS::File::createTemp(tempName)};
    ASSERT_FALSE(f.append("a"));

    // Verify that the file is succesfully moved into the blob object.
    Blob b{move(f)};
    ASSERT_EQ(b.getType(), Blob::Type::FILE);
    ASSERT_EQ(b.getText(), "");
    ASSERT_EQ(string{b.getFile()}, "a");
    ASSERT_EQ(b.size(), 1);
    ASSERT_EQ(b.length(), 1);
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
    ASSERT_EQ(b.size(), 2);
    ASSERT_EQ(b.length(), 2);
  }
  {
    // Truncating a blob with text in memory.
    Blob b{"abc"};
    ASSERT_EQ(b.size(), 3);
    ASSERT_FALSE(b.truncate("hello"));
    ASSERT_EQ(b.size(), 5);
  }
  {
    // Truncating a blob with text in a file.
    Blob b{"abc"};
    b.convertToFile();
    ASSERT_EQ(b.size(), 3);
    ASSERT_FALSE(b.truncate("hello"));
    ASSERT_EQ(b.size(), 5);
  }
}

TEST(Server, Startup) {
  // Verify default state.
  this_thread::sleep_for(quantum);
  ASSERT_EQ(s.getAddress(), "127.0.0.1");
  ASSERT_EQ(s.getPort(), serverPort);
  ASSERT_EQ(s.getErrorCode(), Server::ErrorCode::NO_ERROR);
  ASSERT_EQ(s.getErrorMessage(), "");
  ASSERT_EQ(s.isRunning(), true);

  // Verify that "starting" and already running server does not cause an error.
  s.start();
  ASSERT_EQ(s.getErrorCode(), Server::ErrorCode::NO_ERROR);
  this_thread::sleep_for(quantum);

  // Verify that stopping the server works.
  s.stop();
  ASSERT_EQ(s.isRunning(), false);
  this_thread::sleep_for(quantum);

  // Verify that the address can be changed on a stopped server.
  s.setAddress("0.0.0.0");
  ASSERT_EQ(s.getAddress(), "0.0.0.0");
  ASSERT_EQ(s.getErrorCode(), Server::ErrorCode::NO_ERROR);
  s.setAddress("127.0.0.1");

  // Verify that the port can be changed on a stopped server.
  s.setPort(80);
  ASSERT_EQ(s.getPort(), 80);
  ASSERT_EQ(s.getErrorCode(), Server::ErrorCode::NO_ERROR);
  s.setPort(serverPort);

  // Verify that restarting the server works.
  s.start();
  ASSERT_EQ(s.isRunning(), true);

  // Verify that the address cannot be changed on an already running server.
  s.setAddress("0.0.0.0");
  ASSERT_EQ(s.getAddress(), "127.0.0.1");
  ASSERT_EQ(s.getErrorCode(), Server::ErrorCode::SERVER_ALREADY_RUNNING);
  ASSERT_NE(s.getErrorMessage(), "");

  // Verify that errors are cleared.
  s.clearError();
  ASSERT_EQ(s.getErrorCode(), Server::ErrorCode::NO_ERROR);
  ASSERT_EQ(s.getErrorMessage(), "");

  // Verify that the port cannot be changed on an already running server.
  s.setPort(80);
  ASSERT_EQ(s.getPort(), serverPort);
  ASSERT_EQ(s.getErrorCode(), Server::ErrorCode::SERVER_ALREADY_RUNNING);
  ASSERT_NE(s.getErrorMessage(), "");
  s.clearError();
}

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

TEST(Integration, Simple) {
  {
    Server s{};
    Client c{};
    s.start();
    auto request = make_shared<Message>(Message::Type::REQUEST);
    request
      ->setDomain("127.0.0.1")
      .setPort(s.getPort())
      .setTarget("/foo");
    auto response = c.sendRequest(request);
    response->getReadySemaphore().acquire();

    // Verify the basic Response Message details.
    ASSERT_EQ(response->getTransport(), Message::Transport::FIXED);
    ASSERT_EQ(response->getContentLength(), 12);
    ASSERT_EQ(response->getMessageBody().getType(), Blob::Type::TEXT);
  }
}

TEST(Client, BufferSize) {
  {
    // Verify the response message body is a file-based chunk (because the
    // response is 12 bytes, but we will set the size limit to 10 bytes).
    Server s{};
    Client c{};
    c.setParameter(ClientParameter::MEMCHUNKSIZELIMIT, uint32_t{10});
    s.start();
    auto request = make_shared<Message>(Message::Type::REQUEST);
    request
      ->setDomain("127.0.0.1")
      .setPort(s.getPort())
      .setTarget("/foo");
    auto response = c.sendRequest(request);
    response->getReadySemaphore().acquire();

    // Verify the basic Response Message details.
    ASSERT_EQ(response->getTransport(), Message::Transport::FIXED);
    ASSERT_EQ(response->getContentLength(), 12);
    ASSERT_EQ(response->getMessageBody().getType(), Blob::Type::FILE);
  }
}

int main(int argc, char** argv) {
  s.start();
  serverPort = s.getPort();

  while (0) {
    cout << "listening on " << s.getAddress() << ":" << s.getPort() << endl;

    Client c{};
    auto request = make_shared<Message>(Message::Type::REQUEST);
    request
      ->setDomain("127.0.0.1")
      .setPort(serverPort)
      .setTarget("/foo")
      .addFieldValue("x-test", "hi\"");
    auto response = c.sendRequest(request);
    cout << "Starting wait" << endl;
    response->getReadySemaphore().acquire();
    cout << "After wait" << endl;

    cout << *response;
  }

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

