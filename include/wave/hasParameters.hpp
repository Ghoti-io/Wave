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
 * In order to use this class, the programmer must supply one or two things.
 *   1. An `enum` or `enum class` type (with values defined, of course).
 *   2. (Optional) A function to supply default values.
 *
 * The default value function provided by the class will only provide an empty
 * object, but the programmer may create a subclass to override this behavior.
 *
 * A simple example of the usage of this class can be seen below:
 *
 * @code{cpp}
 * enum class Foo {
 *   GIMME_A_INT,
 *   GIMME_A_STRING,
 * };
 *
 * class FooWithDefaults : public Ghoti::Wave::HasParameters<Foo> {
 *   public:
 *   optional<any> getParameterDefault(const Foo & p) {
 *     if (p == Foo::GIMME_A_INT) {
 *       return int{1};
 *     }
 *     if (p == Foo::GIMME_A_STRING) {
 *       return string{"foo"};
 *     }
 *     return {};
 *   }
 * };
 * @endcode
 *
 * Alternate example of `FooWithDefaults` declaration:
 *
 * @code{cpp}
 * class FooWithDefaults : public Ghoti::Wave::HasParameters<Foo> {
 *   public:
 *   optional<any> getParameterDefault(const Foo & p) {
 *     unordered_map<Foo, optional<any>> defaults{
 *       {Foo::GIMME_A_INT, {int{1}}},
 *       {Foo::GIMME_A_STRING, {string{"foo"}}},
 *     };
 *     return defaults.contains(p) ? defaults[p] : {};
 *   }
 * };
 * @endcode
 *
 *
 * To Use it:
 *
 * @code{cpp}
 * int main() {
 *   FooWithDefaults f{};
 *   cout << *f.getParameter<uint32_t>(Foo::GIMME_A_INT) << endl;
 *   return 0;
 * }
 * @endcode
 *
 * Remember that `getParameter()` returns an `optional<any>` and
 * `getParameters<type>()` returns an `optional<type>`.  In either case,
 * you should verify that the optional value is set before use.
 */
template <typename T>
class HasParameters {
  public:
  /**
   * The constructor.
   */
  HasParameters() : parameterValues{} {}

  /**
   * Provide a default value for the provided parameter key.
   *
   * The default behavior of this function is to only return an empty optional
   * value.  The intent is for this to be overridden by subclasses.
   *
   * @param parameter The parameter key to fetch.
   * @return The associated value.
   */
  virtual std::optional<std::any> getParameterDefault([[maybe_unused]]const T & parameter) {
    return {};
  }

  /**
   * Gets the named parameter if it exists, checking locally first, then
   * checking the global defaults.
   *
   * @param parameter The parameter to get.
   * @return The parameter value if it exists.
   */
  virtual std::optional<std::any> getParameterAny(const T & parameter) {
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
  virtual HasParameters & setParameter(const T & parameter, const std::any & value) {
    this->parameterValues[parameter] = value;
    return *this;
  }

  /**
   * Get the current explicitly-set parameters and their values.
   *
   * @return The current explicitly-set parameters.
   */
  const ParameterMap<T> & getAllParameters() const {
    return this->parameterValues;
  }

  private:
  /**
   * Store explicitly set parameter key/value pairs.
   */
  ParameterMap<T> parameterValues;
};

}

#endif // GHOTI_WAVE_HASPARAMETERS_HPP

