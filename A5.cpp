
#include <limits>
#include <random>
#include <numeric>
#include <cmath>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "A5.hpp"

#include "cs488-framework/MathUtils.hpp"
#include "Utils.hpp"
#include "BSThreadPool.hpp"
#include "Option.hpp"
#include "PhongMaterial.hpp"



// TODO: Refactor refraction information
auto getRefractiveIndices(MediumType m1) -> std::pair<float, float> {
  if(m1 == MediumType::air) return {1., 1.5};
  return {1.5,1.};
}

// TODO Refactor static random function into local stack based MT class
namespace cs488{
  auto randf() -> float {
    static auto rd = std::random_device{};
    static auto mersenne_twister = std::mt19937{rd()};
    static auto dist = std::uniform_real_distribution<float>{0.f, 1.f};
    return dist(mersenne_twister);
  }
}
using cs488::randf;

void A4_Render(SceneNode * root, // What to render  
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
	       ) {

  // for (int i = 0; i < 2; ++ i) {
  //   using namespace glm;
  //   auto out = glm::normalize(glm::vec3{randf(), randf(), randf()});
  //   auto others = perpendicularBasis(out);
  //   log("Basis: ", out, others.first, others.second, '\n');
  // }
  
  debugRender(root, image, eye, view, up, fovy, ambient, lights);
  const size_t h = image.height();
  const size_t w = image.width();


  // Create coordinate space for rays
  const auto minusZNorm = glm::normalize(view);
  const auto xNorm = glm::normalize(glm::cross(view, up));
  const auto yNorm = glm::normalize(up);
  

  const auto aspect = (float)w / (float)h;
  const auto frustumSlopeY = std::tan(degreesToRadians(fovy / 2));

  const auto topLeftRay = minusZNorm
    - (xNorm * aspect * frustumSlopeY)
    + (yNorm * frustumSlopeY);
  
  // Screen vector
  const auto pxWidth = - 2. *topLeftRay.x /  w;
  const auto pxHeight =  2. * topLeftRay.y / h;

  const auto halfPxWidthVec = (xNorm * pxWidth / 2.);
  const auto halfPxHeightVec = - (yNorm * pxHeight / 2.);

  // Calculate a vector that moves a ray from the top left of a pixel to the center
  const auto centerOfPx = halfPxWidthVec + halfPxHeightVec;

  auto threadPool = BS::thread_pool();
  
  for (uint y = 0; y < h; ++y) {
    const auto computeRow =  [w, h, y,
			      eye, ambient, &lights, &root, &image,
			      topLeftRay, xNorm, yNorm, pxWidth, pxHeight, centerOfPx] () -> void {
      for (uint x = 0; x < w; ++x) {

	if(DEBUG_REGION &&
	   (x < DEBUG_REGION_PARAMS.x0 ||
	    x > DEBUG_REGION_PARAMS.x1 ||
	    y < DEBUG_REGION_PARAMS.y0 ||
	    y > DEBUG_REGION_PARAMS.y1))
	  continue;
	  
	logIf(LOG_PX, "(x,y)", x, y);
	     
	// Direction from 0,0 to center of pixel at x,y
	auto directionToCast = topLeftRay
	+ xNorm * pxWidth * x
	- yNorm * pxHeight * y
	+ centerOfPx;
	auto pathToTrace = Ray{eye, directionToCast};

	const auto background = [=]() -> glm::vec3 {
	  //return std::fmod((x+y)*10, 20) > 10  ? glm::vec3{0,0,0} : glm::vec3{1,1,1};
	  return {(float)y/h, .5 + (float) y / h, .5 + (float)y/h};
	};
	  

	auto baseCastFunc = [root, ambient, lights, background](const Ray & rayToCast) -> glm::vec3 {
	  return calculateColor(rayToCast, root, ambient, lights, background, NUM_RECURSIONS, MediumType::air);
	};

	auto applySuperSample = [root, ambient, lights, background,
				 xNorm, yNorm, pxWidth, pxHeight, baseCastFunc](const Ray & rayToCast) -> glm::vec3 {
	  return calculateSupersampleColor(baseCastFunc, rayToCast,
	  {xNorm * pxWidth, yNorm * pxHeight}, randf);
	};
	auto applyDof = [root, ambient, lights, background,
			 xNorm, yNorm, &applySuperSample](const Ray & rayToCast) -> glm::vec3 {
	  return calculateDOFColor(applySuperSample,
				   rayToCast, {xNorm, yNorm}, randf);
	};
	
	auto normalizeColor = 1;
	
	auto finalColor = applyDof(pathToTrace);

	finalColor /= normalizeColor;
	
	image(x, y, 0) = (double) finalColor.x;
	image(x, y, 1) = (double) finalColor.y;
	image(x, y, 2) = (double) finalColor.z;
      }
      std::cout << "donerow" << y << '\n';
    };

    if(!MULTITHREADING) {
      computeRow();
      continue;
    }

    threadPool.push_task(computeRow);
  }
  threadPool.wait_for_tasks();

  if (CELL_SHADING) cellShadeImage(image);
}



auto cellShadeImage(Image & i) -> void {
  logIf(LOG_STAGE, "cellShadeImage");
  // Convert to HSL
  // https://www.rapidtables.com/convert/color/rgb-to-hsl.html

  // Store number of pixels for each V in buckets of size 0.1
  auto vHistogram = std::array<int, 20>{};
  auto updateHistogram = [&vHistogram](double v) -> void {
    int index = std::max(v * 10, 0.);
    vHistogram.at(index) += 1;
  };
  auto printHistogram = [&vHistogram]() -> void {
    for(auto num : vHistogram)
      log("num", num);
  };
  
  for(auto x = 0; x < i.width(); ++x) {
    for(auto y = 0; y < i.height(); ++y) {
      auto r = std::min(i(x,y,0), 1.);
      auto g = std::min(i(x,y,1), 1.);
      auto b = std::min(i(x,y,2), 1.);

      auto [h, s, v] = RGBToHSV(r,g,b);
      // "Bin colors"
      if (LOG_HISTOGRAM)
	updateHistogram(v);

      if(v < .5)
	v = .25;
      else if(v < .75)
	v = .6;
      else
	v = .9;
	
      auto [newR, newG, newB] = HSVToRGB(h,s,v);
      i(x,y,0) = newR;
      i(x,y,1) = newG;
      i(x,y,2) = newB;
    }
  }

  if(!LOG_HISTOGRAM)
    return;
  printHistogram();
}

// TODO Combine calculateColor & Blendshades and rename them
// TODO Combine all intersection into a separate class "IntersectionEngine"
auto calculateColor(Ray path,
		    const SceneNode * root,
		    glm::vec3 ambient,
		    const std::list<Light *> & lights,
		    std::function<glm::vec3()> background,
		    const int numRecursions,
		    const MediumType currentMedium)
  -> glm::vec3 {
  logIf(LOG_STAGE, "calculateColor");
  auto hitRecord = std::vector<HitRecord>{};
  auto didHit = root->intersect(path,hitRecord);

  auto eye = glm::vec3{0., 0., 0.};
  // show background color if no intersections with scene
  if(!didHit)
    return background();

  // Since we hit, we are guaranteed to have 1 intersection at least
  // TODO refactor this into 1 function or something
  const auto [ambientCol, diffuseCol, specularCol] = blendShades(root, path, hitRecord, ambient, lights, background,numRecursions, currentMedium);
  
  return ambientCol + diffuseCol + specularCol;
}

void debugRender(const SceneNode* root,
		 const Image & image,
		 // Viewing parameters  
		 const glm::vec3 & eye,
		 const glm::vec3 & view,
		 const glm::vec3 & up,
		 double fovy,
		 const glm::vec3 & ambient,
		 const std::list<Light *> & lights
		 ) {
  if(!LOG_PARAMS)
    return;
  std::cout << "W22: Calling A4_Render(\n" <<
    "\t" << *root <<
    "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
    "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
    "\t" << "view: " << glm::to_string(view) << std::endl <<
    "\t" << "up:   " << glm::to_string(up) << std::endl <<
    "\t" << "fovy: " << fovy << std::endl <<
    "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
    "\t" << "lights{" << std::endl;
  for(const Light * light : lights) {
    std::cout << "\t\t" <<  *light << std::endl;
  }
  std::cout << "\t}" << std::endl;
  std:: cout <<")" << std::endl;
}


			
auto blendShades(const SceneNode * root,
		 const Ray & path,
		 std::vector<HitRecord> & hits,
		 const glm::vec3 ambient,
		 const std::list<Light*> & lights,
		 std::function<glm::vec3()> background,
		 const int numRecursions,
		 const MediumType currentMedium)
  -> std::tuple<ambientColor, diffuseColor, specularColor> {


  // Assume eye is at origin
  const auto eye = glm::vec3{};
  
  auto ambientResult = glm::vec3{};
  auto diffuseResult = glm::vec3{};
  auto specularResult = glm::vec3{};
  
  auto first_hit = hits.front();
  first_hit.norm = glm::normalize(first_hit.norm);
  
  // Calculate Ambient shading for first hit
  const auto &material = static_cast<const PhongMaterial &>( *first_hit.m );

  const auto isRefractive = material.m_kd.x < 0 || material.m_kd.y < 0 || material.m_kd.z < 0;
  ambientResult += ambient * material.m_kd;
  

  logIf(LOG_STAGE, "Light intersections");
  // Calculate diffuse and specular
  for(const auto * light : lights) {
    const auto directionToLight = glm::normalize(light->position - first_hit.pt);
    const auto hitPtToLight = Ray{first_hit.pt , directionToLight};
    auto lightHitRec = std::vector<HitRecord>{};
    root->intersect(hitPtToLight, lightHitRec);

    // Light was occluded. Do not add lighting
    if(lightHitRec.size() && !SOFT_SHADOWS){
      //diffuseResult += glm::vec3{1,1,1} * -1;
      continue;
    }
    
    // Some shadow rays were not occluded. Calculate percentage
    
    auto percentLight = !SOFT_SHADOWS ? 1. : percentAreaLight(root, light, first_hit.pt, randf);
    if(percentLight < 0.001)
      continue;
    
    const auto distance = glm::length(light->position - first_hit.pt);
    
    auto attenuation = 1. /
      light->falloff[0] +
      light->falloff[1] * distance +
      light->falloff[2] * distance * distance;

    if(std::abs(attenuation - 1) > 0.001)
      logIf(LOG_PX, "non 1 att");
    
    // Diffuse 
    auto diffuseCalc =
      dot(directionToLight, first_hit.norm) *
      attenuation *
      percentLight;
    
    diffuseResult += diffuseCalc * material.m_kd * light->colour;
    // reflect path_ray across intersection norm
    auto specularVector = glm::normalize(-directionToLight
					 + 2 *glm::dot(directionToLight, first_hit.norm)
					 * first_hit.norm);

    auto ptToEye = glm::normalize(eye - first_hit.pt);
	
    auto specularCalc =
      pow(dot(specularVector,ptToEye) , material.m_shininess)
      * attenuation
      * percentLight;

    // Specular result is 0 for some parts under surface, so use max
    specularResult += std::max(specularCalc,0.) * light->colour * material.m_ks;

  }

  if(!numRecursions || !IS_RECURSIVE)
    return {ambientResult, diffuseResult, specularResult};


  logIf(LOG_STAGE, "Reflection");
    //assert(glm::length(first_hit.norm) - 1. < TOLERANCE && "non 1 len norm");
      
  const auto reflectDirectionOut = glm::normalize(path.direction)
    - 2 * glm::dot(glm::normalize(path.direction), first_hit.norm) * first_hit.norm;
  const auto reflectedRay = Ray{first_hit.pt, reflectDirectionOut};
  
  const auto emptyBackground = []{return glm::vec3{0,0,0};};
  // NOTE: Check if we should return early if currentMedium != air
  auto reflectedCol = SPECULAR_REFLECTION? calculateColor(reflectedRay, root, ambient, lights,
							  emptyBackground, // dont reflect bg
							  numRecursions - 1,
							  currentMedium) : glm::vec3{};

  const auto glossyReflectJitterBasis = perpendicularBasis(path.direction, reflectDirectionOut);
  logIf(LOG_STAGE, "Glossy Reflection");
  // TODO REFACTOR GLOSSY REFLECTION
  // TODO get gloss property from material
  const auto GLOSSINESS = .2;
  auto glossyReflectSamples = std::array<glm::vec3, NUM_GLOSSY_REFLECTION_SAMPLES>{};
  for(auto i = 0; i < NUM_GLOSSY_REFLECTION_SAMPLES; ++i) {
    if(!GLOSSY_REFLECTION)
      continue;
    
    const auto jitteredDirection = reflectDirectionOut +
      glm::normalize(glossyReflectJitterBasis.first) * (randf()-.5) * GLOSSINESS+
      glm::normalize(glossyReflectJitterBasis.second) * (randf()-.5) * GLOSSINESS;
    
    const auto newPath = Ray{first_hit.pt, jitteredDirection};

    glossyReflectSamples.at(i) = calculateColor(newPath, root, ambient, lights, emptyBackground, numRecursions - 1, currentMedium);
  }
  auto glossyReflectColor = std::accumulate(glossyReflectSamples.begin(),
					    glossyReflectSamples.end(),
					    glm::vec3{}) / NUM_GLOSSY_REFLECTION_SAMPLES;

  // TODO REFACTOR GLOSSY REFRACTION
  // TODO get refractive index from material

  const auto nonRefractedResult = (specularResult * SPECULAR_WEIGHT +
				   reflectedCol * REFLECT_WEIGHT +
				   glossyReflectColor * GLOSSY_REFLECT_WEIGHT) /
    (SPECULAR_WEIGHT + REFLECT_WEIGHT + GLOSSY_REFLECT_WEIGHT);

  // Get medium's (R)efractive (I)index
  if((!SPECULAR_REFRACTION && !GLOSSY_REFRACTION) || (!isRefractive))
    return {ambientResult, diffuseResult, nonRefractedResult};

  logIf(LOG_STAGE, "Refraction", first_hit, path);
  const auto [currMedRI, nextMedRI] = getRefractiveIndices(currentMedium);
  const auto refractFactor =
    1 - std::pow(currMedRI, 2.) *
    (1 - std::pow(glm::dot(glm::normalize(path.direction), first_hit.norm),
		  2.)) /
    std::pow(nextMedRI, 2.);
  //log(numRecursions, currMedRI, nextMedRI, refractFactor);
  // early return value
  // TODO: Refactor the denominator into separate constexpr factor

  // Total internal reflection
  if(refractFactor < 0.)
    return {ambientResult, diffuseResult, nonRefractedResult};
  
  // Compute refracted ray colors
  auto refractDirection =
    currMedRI / nextMedRI *
    (glm::normalize(path.direction) - first_hit.norm * (glm::dot(glm::normalize(path.direction),
								 first_hit.norm))) -
    first_hit.norm * std::sqrt(refractFactor);
  
  const auto rNought = std::pow( (currMedRI - nextMedRI) / (currMedRI + nextMedRI) , 2);
  const auto cosTheta = std::abs(glm::dot(glm::normalize(path.direction), first_hit.norm));
  
  auto reflectivity = rNought + (1-rNought) * std::pow((1 - cosTheta), 5.);


  const auto refractedRay = Ray{first_hit.pt, refractDirection};
  
  const auto nextMedium = (currentMedium == MediumType::glass) ? MediumType::air : MediumType::glass;

  const auto refractColor = SPECULAR_REFRACTION? calculateColor(refractedRay,
								root,
								ambient,
								lights,
								background,
								numRecursions - 1,
								nextMedium) : glm::vec3{};
  
  
  auto glossyRefractSamples = std::array<glm::vec3, NUM_GLOSSY_REFRACTION_SAMPLES>{};
  const auto glossyRefractJitterBasis = perpendicularBasis(refractDirection);

  for(auto i = 0; i < NUM_GLOSSY_REFRACTION_SAMPLES; ++i) {
    if(!GLOSSY_REFRACTION)
      continue;
    
    const auto jitteredDirection = glm::normalize(refractDirection) +
      glm::normalize(glossyRefractJitterBasis.first) * (randf()-.5) * GLOSSINESS+
      glm::normalize(glossyRefractJitterBasis.second) * (randf()-.5) * GLOSSINESS;
    
    const auto glossRefPath = Ray{first_hit.pt, jitteredDirection};

    glossyRefractSamples.at(i) = calculateColor(glossRefPath,
						root,
						ambient,
						lights,
						background,
						numRecursions - 1,
						nextMedium);
  }
  
  auto glossyRefractColor = std::accumulate(glossyRefractSamples.begin(),
					    glossyRefractSamples.end(),
					    glm::vec3{}) / NUM_GLOSSY_REFRACTION_SAMPLES;

  const auto refractedResult =
    (refractColor * REFRACT_WEIGHT +
     glossyRefractColor * GLOSSY_REFRACT_WEIGHT) / (REFRACT_WEIGHT+GLOSSY_REFRACT_WEIGHT);

    return {ambientResult *  reflectivity ,
	diffuseResult *  reflectivity ,
	(nonRefractedResult) * reflectivity + refractedResult * (1- reflectivity) };
  
}

auto applyReflections(const SceneNode* root,
		      const Light * light,
		      const HitRecord & firstHit); // TODO
		      

auto percentAreaLight(const SceneNode* root,
		      const Light * light,
		      const glm::vec3 hitPoint,
		      std::function<float()> randFunc) -> float {
  
  
  const auto pBasis = perpendicularBasis(light->position - hitPoint);

  auto numHits = 0.;
  
  for(auto i = 0; i < NUM_SHADOW_RAYS; ++i) {
    auto [c1, c2] = std::make_pair(randFunc() - .5, randFunc() - .5);

    // Sample point in unit square
    const auto lightSamplePt = light->position +
      LIGHT_RADIUS * c1 * glm::normalize(pBasis.first) +
      LIGHT_RADIUS * c2 * glm::normalize(pBasis.second);

    const auto shadowRay = Ray{hitPoint , lightSamplePt - hitPoint };
    
    auto hitRecord = std::vector<HitRecord>{};

    root->intersect(shadowRay, hitRecord);

    if(hitRecord.size())
      continue;
    numHits++;
  }
  return numHits/NUM_SHADOW_RAYS;
}

auto calculateSupersampleRay(const Ray & r,
			     std::pair<glm::vec3, glm::vec3> offsetBasis,
			     std::function<float()> randFunc) -> Ray {
  auto jitter =
    offsetBasis.first * (randFunc() - .5) +
    offsetBasis.second * (randFunc()- .5);
  return Ray{r.origin, r.direction + jitter};
}
auto calculateSupersampleColor(std::function<glm::vec3(const Ray& r)> castFunc,
			       const Ray & r,
			       std::pair<glm::vec3, glm::vec3> offsetBasis,
			       std::function<float()> randFunc) -> glm::vec3 {
  if(!SS_ANTI_ALIASING)
    return castFunc(r);
    
  auto samples = std::array<glm::vec3, NUM_SUBPIXELS>{};
  
  for(auto i = 0; i < NUM_SUBPIXELS; ++i) {
    auto newRay = calculateSupersampleRay(r, offsetBasis, randFunc);
    samples.at(i) = castFunc(newRay);
  }

  // Defaults to std::plus<>
  // TODO: switch to std::reduce on newer compiler with it implemented for efficiency
  return std::accumulate(samples.begin(),
			 samples.end(),
			 glm::vec3{}) / NUM_SUBPIXELS;
}

auto calculateDOFRay(const Ray& path,
		     std::pair<glm::vec3, glm::vec3> offsetBasis,
		     std::function<float()> randFunc) -> Ray {
  auto jitter =
    offsetBasis.first * (randFunc() - .5) * LENS_SIZE +
    offsetBasis.second * (randFunc() - .5) * LENS_SIZE;

  const auto newEyePos = path.origin + jitter;
  // https://medium.com/@elope139/depth-of-field-in-path-tracing-e61180417027
  const auto focalDirection = path.direction * FOCAL_LENGTH - jitter;
  
  return {newEyePos, focalDirection };
}

auto calculateDOFColor(std::function<glm::vec3(const Ray& r)> castFunc,
		       const Ray & r,
		       std::pair<glm::vec3, glm::vec3> offsetBasis,
		       std::function<float()> randFunc) -> glm::vec3 {
  if(!DEPTH_OF_FIELD)
    return castFunc(r);

  auto samples = std::array<glm::vec3, NUM_DOF_SAMPLES>{};
  
  for(auto i = 0; i < NUM_DOF_SAMPLES; ++i) {
    auto newRay = calculateDOFRay(r, offsetBasis, randFunc);
    samples.at(i) = castFunc(newRay);
  }

  // Defaults to std::plus<>
  // TODO: switch to std::reduce on newer compiler with it implemented for efficiency
  return std::accumulate(samples.begin(),
			 samples.end(),
			 glm::vec3{}) / NUM_DOF_SAMPLES;
}
		       

// TODO: Write generic sample function for calculate dof/aa/reflect etc
