/**
 * @file
 *
 * Define the Ghoti::Wave::HasServerParameters class.
 */

#include <cstdint>
#include "wave/hasServerParameters.hpp"

using namespace std;
using namespace Ghoti::Util;
using namespace Ghoti::Wave;


ErrorOr<any> HasServerParameters::getParameterDefault(const ServerParameter & p) {
  static unordered_map<ServerParameter, any> defaults{
    {ServerParameter::MAXBUFFERSIZE, {uint32_t{4096}}},
    {ServerParameter::MEMCHUNKSIZELIMIT, {uint32_t{1024 * 1024}}},
  };
  if (defaults.contains(p)) {
    return defaults[p];
  }
  return make_error_code(Util::ErrorCode::PARAMETER_NOT_FOUND);
};

