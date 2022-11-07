// Termm--Fall 2020

#ifndef CS488_A4_HPP
#define CS488_A4_HPP

#include <functional>

#include <glm/glm.hpp>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"

enum Mediums {
  air,
  glass
};


using MediumType = Mediums;

void A4_Render
(
 SceneNode * root,// What to render
 // Image to write to, set to a given width and height
 Image & image,

 // Viewing parameters
 const glm::vec3 & eye,
 const glm::vec3 & view,
 const glm::vec3 & up,
 double fovy,

 // Lighting parameters
 const glm::vec3 & ambient,
 const std::list<Light *> & lights
 );

void debugRender(const SceneNode* root,
		 const Image & image,
		 // Viewing parameters  
		 const glm::vec3 & eye,
		 const glm::vec3 & view,
		 const glm::vec3 & up,
		 double fovy,
		 const glm::vec3 & ambient,
		 const std::list<Light *> & lights);

auto calculateColor(Ray path,
		    const SceneNode * root,
		    glm::vec3 ambient,
		    const std::list<Light *> & lights,
		    std::function<glm::vec3()> background,
		    const int numRecursions,
		    const MediumType currentMedium) -> glm::vec3;


using specularColor = glm::vec3;
using ambientColor = glm::vec3;
using diffuseColor = glm::vec3;

auto blendShades(const SceneNode * root,
		 const Ray & path, // TODO: Refactor to get rid of this
		 std::vector<HitRecord> & hits,
		 glm::vec3 ambient,
		 const std::list<Light*> & lights,
		 std::function<glm::vec3()> background,
		 const int numRecursions,
		 const MediumType currentMedium)
  -> std::tuple<ambientColor, diffuseColor, specularColor>;
  
auto cellShadeImage(Image & i) -> void;

// randFunc must return in [0,1] or [0,1)
auto percentAreaLight(const SceneNode* root,
		      const Light * light,
		      const glm::vec3 hitPoint,
		      std::function<float()> randFunc) -> float;

auto calculateSupersampleColor(std::function<glm::vec3(const Ray& r)> castFunc,
			       const Ray & r,
			       std::pair<glm::vec3, glm::vec3> offsetBasis,
			       std::function<float()> randFunc) -> glm::vec3;

auto calculateDOFColor(std::function<glm::vec3(const Ray& r)> castFunc,
		       const Ray & r,
		       std::pair<glm::vec3, glm::vec3> offsetBasis,
		       std::function<float()> randFunc) -> glm::vec3;

#endif
