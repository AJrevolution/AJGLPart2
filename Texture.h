#pragma once
#include <string>

enum class TextureType
{
	DIFFUSE,
	BASE_COLOR,
	SPECULAR,
	NORMAL,
	HEIGHT,
	SHININESS,
	AMBIENT,
	AMBIENT_OCCLUSION,
	LIGHTMAP,
	UNKNOWN,
};


struct Texture
{
    unsigned int id;
    TextureType type;
    std::string path;
	bool isHDR = false;
};

enum class TextureUnit
{
    Diffuse = 0,
    Normal = 1,
    RoughnessMetallic = 2,
    AmbientOcclusion = 3,
    Irradiance = 4,
    Prefilter = 5,
    BrdfLUT = 6
};
