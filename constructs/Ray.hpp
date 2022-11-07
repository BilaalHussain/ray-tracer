#ifndef CS488_RAY_HPP
#define CS488_RAY_HPP

#include <utility>

#include <glm/glm.hpp>

#include "Utils.hpp"

// POD class
class Ray {
  
public:
  const glm::vec3 origin;
  const glm::vec3 direction;

  // Ray with origin o, direction d
  Ray(const glm::vec3 o,
      const glm::vec3 d):origin{o}, direction{d}{}

  // Unit ray at origin pointing in positive X
  Ray(): origin{}, direction{1,0,0}{log("ray default ctor");}
  friend std::ostream & operator << (std::ostream & os, const Ray & r);
};


// Jitter ray by perpendicular basis with coefficients r1, r2
auto jitter(const Ray & ray,
	    const std::pair<glm::vec3, glm::vec3> & basis,
	    std::pair<float, float> coeffs)
  -> Ray;

// Utility function which returns jittered ray with a seed (generates perpendicular basis)
auto jitter(const Ray & ray,
	   std::pair<float, float> coeffs)
  -> Ray;


//std::ostream & operator << (std::ostream & os, const Ray & r) 

#endif

