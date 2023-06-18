/**
 * @file
 *
 * Define the Ghoti::Wave::Client class.
 */

#include <cstdint>
#include "wave/hasParameters.hpp"

using namespace std;
using namespace Ghoti::Wave;

static ParameterMap globalDefaultValues {
  {Parameter::MAXBUFFERSIZE, uint32_t{4096}},
};

static optional<any> noParameter{};

HasParameters::HasParameters() : defaultParameterValues{globalDefaultValues}, parameterValues{} {}

HasParameters::HasParameters(const ParameterMap & defaultValues) : defaultParameterValues{globalDefaultValues}, parameterValues{defaultValues} {}

optional<any> HasParameters::getParameterAny(Parameter parameter) {
  if (this->parameterValues.contains(parameter)) {
    return this->parameterValues[parameter];
  }
  if (this->defaultParameterValues.contains(parameter)) {
    return this->defaultParameterValues[parameter];
  }
  return noParameter;
}

