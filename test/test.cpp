/**
 * @file
 *
 * Test the general Wave server behavior.
 */

#include<string>
#include<gtest/gtest.h>
#include "wave.hpp"

using namespace std;
using namespace Ghoti::Wave;

Server s{};
uint16_t serverPort = 0;

constexpr auto quantum{10ms};

TEST(Server, Startup){
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

int main(int argc, char** argv) {
  s.start();
  serverPort = s.getPort();

  //while (0) {
    cout << "listening on " << s.getAddress() << ":" << s.getPort() << endl;

    Client c{};
    auto request = make_shared<Message>(Message::Type::REQUEST);
    request
      ->setDomain("127.0.0.1")
      .setPort(serverPort)
      .setTarget("/foo");
    auto response = c.sendRequest(request);
    cout << "Starting wait" << endl;
    response->getReadyFuture().wait();
    cout << "After wait" << endl;

    cout << *response;
  //}

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

