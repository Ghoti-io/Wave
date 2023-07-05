/**
 * @file
 *
 * Define the Ghoti::Wave::HasClientParameters class.
 */

#include <cstdint>
#include "wave/hasClientParameters.hpp"

using namespace std;
using namespace Ghoti::Util;
using namespace Ghoti::Wave;

ErrorOr<any> HasClientParameters::getParameterDefault(const ClientParameter & p) {
  static unordered_map<ClientParameter, any> defaults{
    {ClientParameter::MAXBUFFERSIZE, {uint32_t{4096}}},
    {ClientParameter::MEMCHUNKSIZELIMIT, {uint32_t{1024 * 1024}}},
  };
  if (defaults.contains(p)) {
    return defaults[p];
  }
  return make_error_code(Util::ErrorCode::PARAMETER_NOT_FOUND);
};

