// Termm--Fall 2020

#include "GeometryNode.hpp"

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
	const std::string & name, Primitive *prim, Material *mat )
	: SceneNode( name )
	, m_material( mat )
	, m_primitive( prim )
{
	m_nodeType = NodeType::GeometryNode;
}

void GeometryNode::setMaterial( Material *mat )
{
	// Obviously, there's a potential memory leak here.  A good solution
	// would be to use some kind of reference counting, as in the 
	// C++ shared_ptr.  But I'm going to punt on that problem here.
	// Why?  Two reasons:
	// (a) In practice we expect the scene to be constructed exactly
	//     once.  There's no reason to believe that materials will be
	//     repeatedly overwritten in a GeometryNode.
	// (b) A ray tracer is a program in which you compute once, and 
	//     throw away all your data.  A memory leak won't build up and
	//     crash the program.

	m_material = mat;
}

// TODO: Fix code duplication
bool GeometryNode::intersect(const Ray &r,
			     std::vector<HitRecord>& hr) const {
  
  // On the way down, apply inverse transformation to ray
  auto transformed = Ray{
    glm::mat3(invtrans) * r.origin,
    glm::mat3(invtrans) * r.direction
  };



  auto didHit = m_primitive->intersect(r,hr);
  if(didHit) {
    hr.front().m = m_material;
  }

  for(auto * c : children) 
    didHit |= c->intersect(transformed, hr);

  if(!didHit)
    return didHit;

  // Untransform
  auto & hit = hr.front();
  // hit.norm = glm::mat3(glm::transpose(invtrans)) * hit.norm;
  // hit.pt = glm::vec3(trans * glm::vec4(hit.pt,1.));

  hit.norm = glm::normalize(glm::transpose(glm::mat3(trans)) * hit.norm);
  hit.pt = glm::mat3(trans) * hit.pt;
  return didHit;
}
