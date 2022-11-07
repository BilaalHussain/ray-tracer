// Termm--Fall 2020

#ifndef CS488_MESH_HPP
#define CS488_MESH_HPP

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"

// Use this #define to selectively compile your code to render the
// bounding boxes around your mesh objects. Uncomment this option
// to turn it on.
//#define RENDER_BOUNDING_VOLUMES

struct Triangle
{
  size_t v0;
  size_t v1;
  size_t v2;
  Triangle( size_t pv1, size_t pv2, size_t pv3 ) : v0( pv1 ) , v1( pv2 ) , v2( pv3 )
  {}
};

// A polygonal mesh.
class Mesh : public Primitive {
  std::vector<glm::vec3> m_vertices;
  std::vector<Triangle> m_faces;

  friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
public:
  Mesh(){};
  Mesh( const std::string& fname );
  Mesh( std::vector<glm::vec3> verts, std::vector<Triangle> faces):m_vertices{verts}, m_faces{faces}{};
  virtual bool intersect(const Ray &ray, std::vector<HitRecord>& hr) const override;
};

#endif
