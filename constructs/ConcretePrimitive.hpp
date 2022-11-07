#ifndef CS488_CONCRETE_PRIMITIVE_HPP
#define CS488_CONCRETE_PRIMITIVE_HPP

#include <glm/glm.hpp>
#include "Primitive.hpp"
#include "Mesh.hpp"
#include "Hittable.hpp"
#include "Ray.hpp"

//-------------------------------
class NonhierSphere : public Primitive {
  const glm::vec3 m_pos;
  const double m_radius;
public:
  NonhierSphere(const glm::vec3& pos, double radius) : m_pos(pos), m_radius(radius){}
  virtual ~NonhierSphere() = default;
  virtual bool intersect(const Ray &ray, std::vector<HitRecord>& hr) const override;
};

//-------------------------------
class NonhierBox : public Primitive {
  const glm::vec3 m_pos;
  const double m_size;
  Mesh m_mesh;
public:
  NonhierBox(const glm::vec3& pos, double size);
  virtual ~NonhierBox() = default;
  virtual bool intersect(const Ray &ray, std::vector<HitRecord>& hr) const override;
};

// --------------------------- Sphere
// implicit unit sphere, all transformations done at matrix level
class Sphere : public Primitive {
  NonhierSphere m_sphere;//{{0,0,0}, 1.};
public:
  Sphere():m_sphere{{0,0,0},1.}{};
  virtual ~Sphere() = default;
  virtual bool intersect(const Ray &ray, std::vector<HitRecord>& hr) const override;
};

// --------------------------- Box
class Cube : public Primitive {
  NonhierBox m_box;//{glm::vec3{0,0,0}, 1.};
public:
  Cube():m_box{{0,0,0},1.}{};
  virtual ~Cube() = default;
  virtual bool intersect(const Ray &ray, std::vector<HitRecord>& hr) const override;
};
#endif
