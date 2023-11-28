#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "ResourceManager.h"
#include "PBRHelper.h"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};


struct TextureStateManager
{
	GLuint currentlyBound2D = 0;
	GLuint currentlyBoundCubeMap = 0;

	void bind2D(GLuint tex);
	void bindCubeMap(GLuint tex);
	
};

extern TextureStateManager texState;

//struct IBLTextures
//{
//	GLuint irradianceMap = 0;
//	GLuint prefilterMap = 0;
//	GLuint brdfLUTTexture = 0;
//};


class Mesh
{
public:
	//Mesh Data
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<unsigned int> albedoMapIDs;
	std::vector<unsigned int> normalMapIDs;
	std::vector<unsigned int> roughnessMetallicMapIDs;
	std::vector<unsigned int> aoMapIDs;
	unsigned int VAO;

	
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
	std::vector<unsigned int>& albedoMapIds,
	std::vector<unsigned int>& normalMapIds,
	std::vector<unsigned int>& roughnessMetallicMapIds,
	std::vector<unsigned int>& aoMapIds);
	~Mesh();

	void Draw(const Shader& shader) const;
	void DrawPBR(const Shader& shader, const IBLTextures& iblTextures) const;

	Mesh(Mesh&& other) noexcept;			// Move constructor
	Mesh& operator=(Mesh&& other) noexcept; // Move assignment operator

	Mesh(const Mesh&) = delete;				// Copy constructor
	Mesh& operator=(const Mesh&) = delete;	// Copy assignment operator

private:
	std::vector<unsigned int> textureIDs; // Stores texture IDs for this mesh
	//Render data
	unsigned int VBO, EBO;

	//Initialises all the buffer objects/arrays
	void setupMesh();

	void resetTextures(int startUnit, int endUnit, int irradianceUnit, int prefilterUnit) const;
	void bindTextures(const Shader& shader, const std::vector<unsigned int>& textureIDs, TextureUnit textureUnit, const std::string& uniformName) const;
	void bindIBLTextures(const Shader& shader, const IBLTextures& iblTextures, int irradianceUnit, int prefilterUnit, int brdfLUTUnit) const;

};