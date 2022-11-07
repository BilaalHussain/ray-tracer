#include <algorithm>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "ConcretePrimitive.hpp"
#include "polyroots.hpp"
#include "Mesh.hpp"

constexpr auto MIN_T = 0.01f;
constexpr auto LOG_ROOTS = false;

bool Sphere::intersect(const Ray & r, std::vector<HitRecord>& hr) const{
  return m_sphere.intersect(r,hr);
}

bool NonhierSphere::intersect(const Ray & r, std::vector<HitRecord>& hr)  const {

  // raytracing.github.io/books.html#sendingraysintothescene
  auto eyeToCenter = r.origin - m_pos;
  auto A = (double)glm::dot(r.direction, r.direction);
  auto B = (double)2. *glm::dot(eyeToCenter, r.direction);
  auto C = (double)glm::dot(eyeToCenter, eyeToCenter) - m_radius * m_radius;

  double roots[2];
  auto numRoots = quadraticRoots(A,B,C,roots);
  logIf(LOG_ROOTS && numRoots , r, '\n', m_pos, m_radius, numRoots, '\n',roots[0], roots[1], '\n');
  //logFrame(m_radius, glm::to_string(m_pos));
  if(numRoots == 0)
    return false;

  // Single degenerate root
  if(numRoots == 1 && roots[0] < MIN_T)
    return false;

  auto t{roots[0]};

  // Pick 
  if(numRoots == 2)
    t = std::min({roots[0] <= MIN_T ? std::numeric_limits<double>::max() : roots[0],
	  roots[1] <= MIN_T ? std::numeric_limits<double>::max() : roots[1]});
  logIf(LOG_ROOTS && numRoots, "after filter", numRoots, t, roots[0], roots[1]);
  // All roots are degerate
  if(t <= MIN_T || t >= std::numeric_limits<double>::max())
    return false;
  logIf(LOG_ROOTS && numRoots, "not return false lol");
  // if the existing intersection is closer
  if(hr.size() && hr.front().t < t)
    return false;


  auto hitPt = r.origin + r.direction * t;

  // difference from center of sphere
  auto normal = hitPt - m_pos;

  // try to 'efficiently' (lol) update hit record
  if(!hr.size())
    hr.emplace_back(hitPt, normal, (float)t, nullptr);
  else
    hr.front()= HitRecord{hitPt, normal, (float)t, nullptr};
  
  return true;
}

bool Cube::intersect(const Ray & r, std::vector<HitRecord>& hr) const{
  return m_box.intersect(r,hr);
}

bool NonhierBox::intersect(const Ray & r, std::vector<HitRecord>& hr) const{
  
  return m_mesh.intersect(r,hr);
}

NonhierBox::NonhierBox(const glm::vec3& pos, double size): m_pos(pos), m_size(size) {

  
  auto verts = std::vector<glm::vec3> {{
      {-.5, -.5, -.5}, // bot left back 0
      {-.5, .5, -.5},  // top left back 1
      {.5, -.5, -.5},  // bot right back 2
      {.5, .5, -.5},  // top right back 3
      {-.5, -.5, .5}, // bot left front 4
      {-.5, .5, .5},  // top left front 5
      {.5, -.5, .5},  // bot right front 6
      {.5, .5, .5},  // top right front 7
    }};
  for(auto & pt : verts) {
    pt *= size;
    pt += pos;
  }
  auto faces = std::vector<Triangle>{{
      //back
      Triangle{0,1,3},
      Triangle{0,3,2},
      //left
      Triangle{0,5,1},
      Triangle{0,4,5},
      //bot
      Triangle{0,2,6},
      Triangle{0,6,4},
      //top
      Triangle{7,2,3},
      Triangle{7,6,2},
      //top
      Triangle{7,3,1},
      Triangle{7,1,5},
      //front
      Triangle{7,4,6},
      Triangle{7,5,4},
    }};

  m_mesh = Mesh{verts,faces};
}
