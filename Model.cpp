#include "Model.h"

Model::Model(std::string const& directoryOfModel, std::string const& modelPath, std::shared_ptr<ResourceManager> rManager, bool gamma)
	: gammaCorrection(gamma),
	resourceManager(rManager),
	directory(directoryOfModel)
{
	loadModel(modelPath);
	setPosition(glm::vec3(0, 0, 0));
	setScale(glm::vec3(1, 1, 1));
	setRotation(glm::vec3(0, 0, 0));

}

void Model::DrawPBR(const Shader& shader, const IBLTextures& iblTextures) const
{
	for (const auto& mesh : meshes)
		mesh.DrawPBR(shader, iblTextures);
}

void Model::UpdateTransform()
{
	modelMatrix = glm::mat4(1.0f); // Reset to identity matrix
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::scale(modelMatrix, scale);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
}

void Model::setPosition(const glm::vec3& newPosition)
{
	position = newPosition;
	UpdateTransform();
}

void Model::setScale(const glm::vec3& newScale)
{
	scale = newScale;
	UpdateTransform();
}

void Model::setRotation(const glm::vec3& newRotation)
{
	rotation = newRotation;
	UpdateTransform();
}


void Model::loadModel(std::string const& path)
{
	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);//
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}
	// retrieve the directory path of the filepath
	modelPath = path.substr(0, path.find_last_of('/'));

	// process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// after processing all of the meshes, recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// data to fill
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<unsigned int> albedoMapIDs;
	std::vector<unsigned int> normalMapIDs;
	std::vector<unsigned int> roughnessMetallicMapIDs;
	std::vector<unsigned int> aoMapIDs;
	//std::vector<Texture> textures;

	// Walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
						  // positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		// texture coordinates
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		// tangent
		vector.x = mesh->mTangents[i].x;
		vector.y = mesh->mTangents[i].y;
		vector.z = mesh->mTangents[i].z;
		vertex.Tangent = vector;
		// bitangent
		vector.x = mesh->mBitangents[i].x;
		vector.y = mesh->mBitangents[i].y;
		vector.z = mesh->mBitangents[i].z;
		vertex.Bitangent = vector;
		vertices.push_back(vertex);
	}
	// now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	albedoMapIDs = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "albedoMap");
	normalMapIDs = loadMaterialTextures(material, aiTextureType_NORMALS, "normalMap");
	roughnessMetallicMapIDs = loadMaterialTextures(material, aiTextureType_UNKNOWN, "roughnessMetallicMap");
	aoMapIDs = loadMaterialTextures(material, aiTextureType_LIGHTMAP, "aoMap");

	if (albedoMapIDs.empty()) 
	{ 
		unsigned int defaultAlbedoID = ResourceManager::CreateDefaultTexture(128, 128, 128);
		albedoMapIDs.push_back(defaultAlbedoID); 
	}
	if (normalMapIDs.empty())
	{
		unsigned int defaultNormalID = ResourceManager::CreateDefaultTexture(128, 128, 255);
		normalMapIDs.push_back(defaultNormalID);
	}
	if (roughnessMetallicMapIDs.empty())
	{
		unsigned int defaultRoughnessID = ResourceManager::CreateDefaultTexture(0, 0, 0);
		roughnessMetallicMapIDs.push_back(defaultRoughnessID);
	}

	if (aoMapIDs.empty())
	{
		unsigned int defaultAOID = ResourceManager::CreateDefaultTexture(255, 255, 255);
		aoMapIDs.push_back(defaultAOID);
	}

	// return a mesh object created from the extracted mesh data
	return Mesh(vertices, indices, albedoMapIDs, normalMapIDs, roughnessMetallicMapIDs, aoMapIDs);

}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
std::vector<unsigned int> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const std::string& customPath)
{
	std::vector<unsigned int> textureIDs;
	aiString currentPath;
	unsigned int textureCount = mat->GetTextureCount(type);

	// The original functionality: loop through all textures of the given type and load them
	for (unsigned int i = 0; i < textureCount; i++)
	{
		mat->GetTexture(type, i, &currentPath);
		Texture texture = resourceManager->getTexture(currentPath.C_Str(), this->directory, type);
		textureIDs.push_back(texture.id);
	}

	return textureIDs;
}
