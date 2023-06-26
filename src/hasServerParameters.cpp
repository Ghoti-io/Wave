/**
 * @file
 *
 * Define the Ghoti::Wave::HasServerParameters class.
 */

#include <cstdint>
#include "wave/hasServerParameters.hpp"

using namespace std;
using namespace Ghoti::Wave;


optional<any> HasServerParameters::getParameterDefault(const ServerParameter & p) {
  static unordered_map<ServerParameter, any> defaults{
    {ServerParameter::MAXBUFFERSIZE, {uint32_t{4096}}},
    {ServerParameter::MEMCHUNKSIZELIMIT, {uint32_t{1024 * 1024}}},
  };
  if (defaults.contains(p)) {
    return defaults[p];
  }
  return {};
};

