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
 *
 * HasParameters is a templated utility class.  It's purpose is to associate
 * key/value pairs as settings, in which the keys are of an enum type and the
 * values may be of any type.
 *
 * In order to use this class, the programmer must supply two things.
 *   1. An `enum` or `enum class` type (with values defined, of course).
 *   2. A function to supply default values.
 *
 * The default value function must be defined by the programmer.  The prototype
 * is defined in the header file via the template, but the definition must be
 * written, even if the definition only returns a default (empty) value.
 *
 * A simple example of the usage of this class can be seen below:
 *
 * @code{cpp}
 * enum class Foo {
 *   GIMME_A_INT,
 *   GIMME_A_STRING,
 * };
 *
 * template<>
 * optional<any> Ghoti::Wave::HasParameters<Foo>::getParameterDefault(const Foo & p) {
 *   if (p == Foo::GIMME_A_INT) {
 *     return int{1};
 *   }
 *   if (p == Foo::GIMME_A_STRING) {
 *     return string{"foo"};
 *   }
 *   return {};
 * }
 * @endcode
 *
 * Alternate example of `HasParameters`:
 *
 * @code{cpp}
 * template<>
 * optional<any> Ghoti::Wave::HasParameters<Foo>::getParameterDefault(const Foo & p) {
 *   unordered_map<Foo, optional<any>> defaults{
 *     {Foo::GIMME_A_INT, {int{1}}},
 *     {Foo::GIMME_A_STRING, {string{"foo"}}},
 *   };
 *
 *   return defaults.contains(p) ? defaults[p] : {};
 * }
 * @endcode
 *
 *
 * To Use it:
 *
 * @code{cpp}
 * class Something : public HasParameters<Foo> {}
 * int main() {
 *   Something s{};
 *   cout << s.getParameter<uint32_t>(Foo::GIMME_A_INT) << endl;
 *   return 0;
 * }
 * @endcode
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
   * This is only a prototype.  The actual function must be supplied by the
   * programmer in order to implement the desired default values.  See the
   * example in the class documentation.
   *
   * This definition declares the function prototype which the programmer must
   * then supply.  The programmer does not need to redeclare the prototype
   * itself, but only needs to make sure that the function definition is
   * implemented as part of the standard compilation step.
   *
   * @param parameter The parameter key to fetch.
   * @return The associated value.
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

