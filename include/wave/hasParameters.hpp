/**
 * @file
 *
 * Header file for declaring the hasParameters class.
 */

#ifndef GHOTI_WAVE_HASPARAMETERS_HPP
#define GHOTI_WAVE_HASPARAMETERS_HPP

#include <any>
#include <optional>
#include <unordered_map>
#include "wave/macros.hpp"

namespace Ghoti::Wave {

/**
 * A type alias for the structure that stores the settings map.
 */
using ParameterMap = std::unordered_map<Parameter, std::any>;

/**
 * Serves as a base class for any other class to have settings parameters.
 */
class HasParameters {
  public:
  /**
   * The constructor.
   */
  HasParameters();

  /**
   * The constructor.
   *
   * Create a parameter map with initial values.
   *
   * @param defaultValues The initial settings to be used.
   */
  HasParameters(const ParameterMap & defaultValues);

  /**
   * Gets the named parameter if it exists, checking locally first, then
   * checking the global defaults.
   *
   * @param parameter The parameter to get.
   * @return The parameter value if it exists.
   */
  std::optional<std::any> getParameterAny(Parameter);

  /**
   * Get the parameter as a specified type.
   *
   * The result is returned as an optional.  If there is no parameter value,
   * then the optional value will be false.
   *
   * @param parameter The parameter value to get.
   * @return The (optional) parameter value.
   */
  template<class T>
  const std::optional<T> getParameter(Parameter parameter) {
    auto val = getParameterAny(parameter);
    if (val && (val->type() == typeid(T))) {
      return {std::any_cast<const T &>(*val)};
    }
    return {};
  }

  /**
   * Set a parameter.
   *
   * @param parameter The parameter key to be set.
   * @param value The parameter value to be set.
   * @return The calling object, to allow for chaining.
   */
  template<class T>
  HasParameters & setParameter(Parameter parameter, const std::any & value) {
    this->parameterValues[parameter] = value;
    return *this;
  }

  private:
  /**
   * Store default parameter key/value pairs, if the value is not otherwise
   * explicitly set.
   */
  ParameterMap defaultParameterValues;

  /**
   * Store explicitly set parameter key/value pairs.
   */
  ParameterMap parameterValues;
};

}

#endif // GHOTI_WAVE_HASPARAMETERS_HPP

