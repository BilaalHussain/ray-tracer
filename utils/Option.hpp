/*
1  Soft shadows ---------------- Done
2  depth of field -------------- Done
3  glossy reflection ----------- Done
4  glossy refraction ----------- Done
5  recursive reflection -------- Done
6  specular reflection --------- Done
7  specular refraction --------- Done
8  stochastic AA ------------- Done
9  cell shading --------------- Done
10  cylinder and cone primitive

*/
#ifndef CS488_OPTION_HPP
#define CS488_OPTION_HPP

// DEBUGGING {
typedef struct DebugRegion{
  uint x0,x1,y0,y1;
  constexpr DebugRegion(uint x0, uint x1, uint y0, uint y1):x0{x0}, x1{x1}, y0{y0}, y1{y1}{}
} DebugRegion;
constexpr auto DEBUG_REGION = false;
constexpr auto DEBUG_REGION_PARAMS = DebugRegion{113,114,132,133};
constexpr auto LOG_STAGE = false && !LOG_NONE;
constexpr auto LOG_PARAMS = false && !LOG_NONE;
constexpr auto LOG_HISTOGRAM = false && !LOG_NONE;
constexpr auto LOG_PX = true && !LOG_NONE;
// } DEBUGGING

// CELL SHADING {
constexpr auto CELL_SHADING = false;
// } CELL SHADING

// MULTI THREADING {
constexpr auto MULTITHREADING = true;
// } MULTI THREADING

// SOFT SHADOWS { 
constexpr auto SOFT_SHADOWS = false;
constexpr auto NUM_SHADOW_RAYS = 20;
constexpr auto LIGHT_RADIUS = 30; // LIGHT_RADIUS = Square side length for distorting light
// } SOFT SHADOWS

// ANTI ALIASING {
constexpr auto SS_ANTI_ALIASING = false;
constexpr auto NUM_SUBPIXELS = 10;
// } ANTI ALIASING

// DEPTH OF FIELD {
constexpr auto DEPTH_OF_FIELD = false;
constexpr auto LENS_SIZE = 30;
constexpr auto FOCAL_LENGTH = 600.;
constexpr auto NUM_DOF_SAMPLES = 10;
// } DEPTH OF FIELD

constexpr auto SPECULAR_REFLECTION = false;
constexpr auto GLOSSY_REFLECTION = false;
constexpr auto SPECULAR_REFRACTION = true;
constexpr auto GLOSSY_REFRACTION = true;
constexpr auto IS_RECURSIVE = SPECULAR_REFLECTION ||
  GLOSSY_REFLECTION || SPECULAR_REFRACTION || GLOSSY_REFRACTION;

constexpr auto SPECULAR_WEIGHT = 1.;

constexpr auto REFLECT_WEIGHT = SPECULAR_REFLECTION ? .8 : 0.;
constexpr auto GLOSSY_REFLECT_WEIGHT = GLOSSY_REFLECTION ? .4 : 0.;

constexpr auto REFRACT_WEIGHT = SPECULAR_REFRACTION ? .2 : 0.;
constexpr auto GLOSSY_REFRACT_WEIGHT = GLOSSY_REFRACTION ? .3 : 0.;  

constexpr auto SPECULAR_RECURSIVE_TOTAL_WEIGHT =
  SPECULAR_WEIGHT +
  REFLECT_WEIGHT + GLOSSY_REFLECT_WEIGHT +
  REFRACT_WEIGHT + GLOSSY_REFRACT_WEIGHT;

constexpr auto NUM_GLOSSY_REFLECTION_SAMPLES = 10;
constexpr auto NUM_GLOSSY_REFRACTION_SAMPLES = 100;

constexpr auto NUM_RECURSIONS = 2;

#endif
