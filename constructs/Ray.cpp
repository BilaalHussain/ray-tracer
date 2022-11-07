#include "Ray.hpp"


std::ostream & operator << (std::ostream & os, const Ray & r){
  os << "Ray{" <<
    "o " << glm::to_string(r.origin) << ", " <<
    "d" << glm::to_string(r.direction) << "}";
  return os;
}


auto jitter(const Ray & ray,
	   const std::pair<glm::vec3, glm::vec3> & basis,
	   std::pair<float, float> coeffs) -> Ray {

  auto newDestination = ray.direction
    + basis.first * coeffs.first
    + basis.second * coeffs.second;
  
  return Ray{ray.origin, newDestination};
}

auto jitter(const Ray & ray,
	    std::pair<float, float> coeffs) -> Ray{
  auto bases = perpendicularBasis(ray.direction);
  
  return jitter(ray, bases, coeffs);

}




