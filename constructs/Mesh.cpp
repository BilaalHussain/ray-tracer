// Termm--Fall 2020

#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

Mesh::Mesh( const std::string& fname )
  : m_vertices()
  , m_faces()
{
  std::string code;
  double vx, vy, vz;
  size_t s1, s2, s3;

  std::ifstream ifs( fname.c_str() );
  while( ifs >> code ) {
    if( code == "v" ) {
      ifs >> vx >> vy >> vz;
      m_vertices.push_back( glm::vec3( vx, vy, vz ) );
    } else if( code == "f" ) {
      ifs >> s1 >> s2 >> s3;
      m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
    }
  }
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
    for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
    const MeshVertex& v = mesh.m_verts[idx];
    out << glm::to_string( v.m_position );
    if( mesh.m_have_norm ) {
    out << " / " << glm::to_string( v.m_normal );
    }
    if( mesh.m_have_uv ) {
    out << " / " << glm::to_string( v.m_uv );
    }
    }

  */
  out << "}";
  return out;
}


constexpr auto CULLING = true;
auto rayTriangleIntersect(const Ray&ray,
			  std::vector<HitRecord>& hr,
			  const Triangle tri,
			  const std::vector<glm::vec3>& verts) -> bool {

  // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection

  auto v0 = verts.at(tri.v0);
  auto v1 = verts.at(tri.v1);
  auto v2 = verts.at(tri.v2);
  
  auto v0v1 = v1 - v0;
  auto v0v2 = v2 - v0;

  auto pvec = glm::cross(ray.direction, v0v2);
  auto determinant = glm::dot(v0v1, pvec);

  // Cull backface 
  if (CULLING && determinant < TOLERANCE) return false;
  // Dont cull backface 
  if (!CULLING && std::fabs(determinant) < TOLERANCE) return false; 


  // Find p
  auto invDeterminant = 1. / determinant;


  // Check if intersection point P can be formed by linear combo of edges

  auto tvec = ray.origin - v0;
  auto u = glm::dot(tvec, pvec) * invDeterminant;
  if(u < 0 || u > 1) return false;

  auto qvec = glm::cross(tvec, v0v1);
  auto v = glm::dot(ray.direction, qvec) * invDeterminant;
  if(v < 0 || u + v > 1) return false;

  // Intersection point P CAN be formed by linear combo of edges
  auto t = glm::dot(v0v2, qvec) * invDeterminant;

  // Intersection point is close to the origin of the ray. Probably erronous intersection
  if( t < TOLERANCE )
    return false;

  if(hr.size() && hr.front().t < t)
    return false;

  auto hitPt = ray.origin + ray.direction * t;
  auto normal = glm::cross(v0v1, v0v2);

  if(!hr.size())
    hr.emplace_back(hitPt, normal, (float)t, nullptr);
  else
    hr.front() = HitRecord{hitPt, normal, (float)t, nullptr};
  return true;
}

bool Mesh::intersect(const Ray &ray,
		     std::vector<HitRecord>& hr) const {
  
  for(auto triangle : m_faces) {
    if(rayTriangleIntersect(ray,
			    hr,
			    triangle,
			    m_vertices))
      return true;
  }
  return false;
}


