#ifndef CS488_HITTABLE_HPP
#define CS488_HITTABLE_HPP

#include <optional>
#include <vector>

#include "Ray.hpp"
#include "Material.hpp"

typedef struct HitRecord {
  glm::vec3 pt;
  glm::vec3 norm;
  // t in implicit formula for ray
  float t;
  Material * m;
  HitRecord(glm::vec3 pt, glm::vec3 norm, float t, Material * m):
    pt{pt}, norm{norm}, t{t}, m{m}{}
  HitRecord(){}

    friend std::ostream & operator << (std::ostream & os, const HitRecord & hr){
  os << "HitRecord{" <<
    "pt " << glm::to_string(hr.pt) << ", " <<
    "norm" << glm::to_string(hr.norm) << ", " <<
    "}\n";
    return os;
  }
} HitRecord;

// TODO: Better intersection class
// template <class HitRecord>
class Intersectable {
  virtual bool intersect(const Ray &r,
			 std::vector<HitRecord>& hr)
    const
    = 0;
};

#endif
