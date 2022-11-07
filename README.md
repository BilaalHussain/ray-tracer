# Compilation
- `premake4 gmake && make && cd Assets && ../A5 <FILENAME>` where filename is a lua script Assets folder
the format of a4 is used for this project

-- I.e. do not prefix filename with /Assets
-- c++17 used
- tested on cs488 vm

# Manual

- Features can be enabled from Option.hpp, which requires recompiling the program
the following features have boolean flags to enable/disable them
CELL_SHADING|SOFT_SHADOWS|SS_ANTI_ALIASING|DEPTH_OF_FIELD|
SPECULAR_REFLECTION|GLOSSY_REFLECTION|SPECULAR_REFRACTION|GLOSSY_REFRACTION

each flag is independent and can be combined. Combining multiple features exponentially increases render time

The amount of samples for glossy reflection, glossy refraction, soft shadows, depth of field, can be changed from their respective flags in option.hpp

also, the recursive depth for reflections/refractions can be changed with the NUM_RECURSIONS flag
also, the relative weight between glossy/reflectiveness can be controlled with the flags. this changes how the specular vs glossyness rays are combined

the lens size, focal length can be changed for depth of field

the light size and number of samples can be changed for soft shadows

I did not implement cone/sphere primitives


1  Soft shadows ---------------- Done (softShadows.png)
2  depth of field -------------- Done (DOF.png)
3  glossy reflection ----------- Done (glossyReflection.png)
4  glossy refraction ----------- Done (glossyRefraction.png)
5  recursive reflection -------- Done 
6  specular reflection --------- Done (specularRefraction.png
7  specular refraction --------- Done (specularReflection.png)
8  supersample AA --------------- Done (AA.png)
9  cell shading ---------------- Done (cellShaded.png)
10 cylinder and cone primitive - Not done 

- rather than implement anti-aliasing with linear supersampling, i chose to implement it with stochastic AA
