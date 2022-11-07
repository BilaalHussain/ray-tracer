#include "Utils.hpp"
#include <algorithm>
#include <tuple>

// get perpendicular basis for a incoming and reflected ray
auto perpendicularBasis(const glm::vec3 in, const glm::vec3 out) -> std::pair<glm::vec3, glm::vec3> {
  auto b1 = glm::cross(in, out);
  auto b2 = glm::cross(b1, out);
  return {b1, b2};
}

auto perpendicularBasis(const glm::vec3 out) -> std::pair<glm::vec3, glm::vec3> {
  auto b1 = glm::vec3{0, -out.z, out.y};
  auto b2 = glm::cross(b1, out);
  return {b1, b2};
}

auto RGBToHSV(double r,
	      double g,
	      double b) -> std::tuple<double, double, double> {


  // tolerance for delta
  constexpr auto TOLERANCE = 0.001;
  
  auto cMax = std::max({r, g, b});
  auto cMin = std::min({r, g, b});

  auto cDelta = cMax - cMin;

  auto V = cMax;

  auto cMaxIsZero = cMax < TOLERANCE;
  
  auto S =  cMaxIsZero ? 0. : cDelta / cMax ;

  if(cMaxIsZero)
    return {0., S, V};

  if (cMax == r)
    return { 60. * std::fmod((g-b) / cDelta + 360., 6.), S, V };
  if (cMax == g)
    return { (60. * ((b-r) / cDelta)+2), S, V };
  if (cMax == b)
    return { (60. * ((g-b) / cDelta)+4), S, V };

  assert(false && "HSV conversion failed");
}

auto HSVToRGB(const double h,
	      const double s,
	      const double v) -> std::tuple<double, double, double> {
  // Using https://www.rapidtables.com/convert/color/hsv-to-rgb.html

  auto c = v * s;
  auto x = c * (1 - std::abs(std::fmod(h/60., 2) - 1));
  auto m = v - c;

  //assert(h >= 0 && h < 360. && "Invalid hue");

  c +=m;
  x +=m;
  
  if (h < 60.)
    return {c, x, m};

  if(h < 120.)
    return {x,c,m};

  if(h < 180.)
    return {m,c,x};

  if(h < 240.)
    return {m,x,c};
  
  if(h < 300.)
    return {x,m,c};
  
  if(h < 360.)
    return {c,m,x};
}
