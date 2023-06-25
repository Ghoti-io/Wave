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

namespace Ghoti::Wave {

/**
 * A type alias for the structure that stores the settings map.
 */
template <typename T>
using ParameterMap = std::unordered_map<T, std::any>;

/**
 * Serves as a base class for any other class to have settings parameters.
 */
template <typename T>
class HasParameters {
  public:
  /**
   * The constructor.
   */
  HasParameters() : parameterValues{} {}

  /**
   * The constructor.
   *
   * Create a parameter map with initial values.
   *
   * @param defaultValues The initial settings to be used.
   */
  HasParameters(const ParameterMap<T> & defaultValues) : parameterValues{defaultValues} {}

  /**
   * This function must be supplied by the programmer.
   */
  static std::optional<std::any> getParameterDefault(const T & parameter);

  /**
   * Gets the named parameter if it exists, checking locally first, then
   * checking the global defaults.
   *
   * @param parameter The parameter to get.
   * @return The parameter value if it exists.
   */
  std::optional<std::any> getParameterAny(const T & parameter) {
    if (this->parameterValues.contains(parameter)) {
      return this->parameterValues[parameter];
    }
    return getParameterDefault(parameter);
  }


  /**
   * Get the parameter as a specified type.
   *
   * The result is returned as an optional.  If there is no parameter value,
   * then the optional value will be false.
   *
   * @param parameter The parameter value to get.
   * @return The (optional) parameter value.
   */
  template<class U>
  const std::optional<U> getParameter(const T & parameter) {
    auto val = getParameterAny(parameter);
    if (val && (val->type() == typeid(U))) {
      return {std::any_cast<const U &>(*val)};
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
  HasParameters & setParameter(const T & parameter, const std::any & value) {
    this->parameterValues[parameter] = value;
    return *this;
  }

  private:
  /**
   * Store explicitly set parameter key/value pairs.
   */
  ParameterMap<T> parameterValues;
};

}

#endif // GHOTI_WAVE_HASPARAMETERS_HPP

