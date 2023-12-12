#pragma once

//#include <map>
#include <glad/glad.h>
#include "Texture.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include <assimp/pbrmaterial.h>
#include "stb_image.h"
#include <unordered_map>

class ResourceManager
{
public:
	Texture getTexture(const std::string& path, const std::string& directory, aiTextureType type, bool isHDR = false);
	static std::string TextureTypeToString(TextureType type);
	static unsigned int CreateDefaultTexture(unsigned char r, unsigned char g, unsigned char b);
	
	//Static method to initialize anisotropy level
	static void InitMaxAnisotropy();
private:
	std::unordered_map<std::string, Texture> textures;
	Texture loadTextureFromFile(const std::string& path, const std::string& directory, TextureType type, bool isHDR = false);
	TextureType aiTextureTypeToTextureType(aiTextureType type);

	static GLfloat maxAnisotropy;
};

