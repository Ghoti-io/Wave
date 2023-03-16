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

string request {R"(GET /hello.html HTTP/1.1
Host: 0.0.0.0
Accept-Language: en)"};

string response {R"(HTTP/1.1 200 OK
Server: Hello
Content-Length: 13
Content-Type: text/plain

Hello, world)"};

TEST(Simple, Simple){

}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

