#pragma once

#include "Mesh.h"
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//#include <assimp/GltfMaterial.h>
//#include <assimp/pbrmaterial.h>
//#include "stb_image.h"
#include <map>

//unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false, bool useAbsolutePath = false);
//TextureType aiTextureTypeToTextureType(aiTextureType type);

struct MaterialTexturePaths
{
	std::vector<std::string> diffuse;
	std::vector<std::string> normal;
	std::vector<std::string> roughness;
	std::vector<std::string> ao;
	std::vector<std::string> metallic;
};



class Model
{
public:
	/*  Model Data */

	std::vector<Mesh> meshes;
	std::string directory;
	std::string modelPath;
	bool gammaCorrection = false;
	//bool pbr = true;

	IBLTextures iblTextures;

	//static GLuint irradianceMap;
	//static GLuint prefilterMap;
	//static GLuint brdfLUTTexture;

	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;
	glm::mat4 modelMatrix;

	Model(std::string const& directoryOfModel, std::string const& modelPath, std::shared_ptr<ResourceManager> rManager, bool gamma = false);

	void DrawPBR(const Shader& shader, const IBLTextures& iblTextures) const;
	void UpdateTransform();

	void setPosition(const glm::vec3& newPosition);
	void setScale(const glm::vec3& newScale);
	void setRotation(const glm::vec3& newRotation);


private:
	std::shared_ptr<ResourceManager> resourceManager;
	/*  Functions   */
	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(std::string const& path);

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene);

	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	std::vector<unsigned int> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const std::string& customPath = "");

};

