#ifndef CS488_PRIMITIVE_HPP
#define CS488_PRIMITIVE_HPP

#include <glm/glm.hpp>
#include "Hittable.hpp"
#include "Ray.hpp"

// Pure virtual
class Primitive : public Intersectable {
public:
  virtual ~Primitive() = default;
  virtual bool intersect(const Ray &ray, std::vector<HitRecord>& hr) const =0;
};

#endif
