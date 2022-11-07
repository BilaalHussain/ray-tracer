#ifndef CS488_UTILS_HPP
#define CS488_UTILS_HPP

#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <type_traits>
#include <utility>
#include <tuple>

#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include "cs488-framework/MathUtils.hpp"

constexpr static auto LOG_NONE{true};
constexpr static auto TOLERANCE = 0.001;

// Use SFINAE to avoid redeclaring ostream operator for non GLM types
template <typename GLMType, typename = decltype(glm::to_string(std::declval<GLMType>()))>
std::ostream& operator<<(std::ostream& out, const GLMType& g)
{
    return out << glm::to_string(g);
}

inline bool shouldLogFrame{true};

template<typename T>
std::string to_string_impl(const T& t)
{
  std::stringstream ss;
  ss << t;
  return ss.str();
}

template<typename ...Args>
std::vector<std::string> many_to_string_impl(const Args & ...args)
{
  return {to_string_impl(args)...};
}

template<typename ...Args>
void log(const Args & ...args){
  if(LOG_NONE)
    return;

  const auto v = many_to_string_impl(args...);
  for(const auto &o:v)
    std::cout<< o << ", ";
  std::cout << '\n';
}
template<typename ...Args>
void logFrame(const Args & ...args){
  if(!shouldLogFrame)
    return;
  log(args...);
}

template<typename ...Args>
void logIf(bool flags, const Args & ...args){
  if(!flags)
    return;
  log(args...);
}

// uses out as one of the bases
// uses in to compute bases
auto perpendicularBasis(const glm::vec3 in, const glm::vec3 out)
  -> std::pair<glm::vec3, glm::vec3>;

// uses out as one of the bases
auto perpendicularBasis(const glm::vec3 out)
  -> std::pair<glm::vec3, glm::vec3>;

// Assume r, g, b are from [0,1]
auto RGBToHSV(const double r, const double g, const double b) -> std::tuple<double, double, double>;
// Assume that S, V are in decimal notation for percent
auto HSVToRGB(const double h, const double s, const double v) -> std::tuple<double, double, double>;
#endif
