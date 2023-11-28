#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Texture.h"

struct CameraData
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 camPos;
	float padding;
};

//std::string TextureTypeToString(TextureType type);

class Shader
{
public:
	unsigned int ID;
	GLuint uboCamera, uboGlobal;

	//Constructor & Deconstructor
	Shader() = delete;
	Shader(const char* vertexPath, const char* fragmentPath);
	~Shader();
	void use();

	//Utility uniform functions
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec3(const std::string& name, const glm::vec3& value) const;
	void setMat4(const std::string& name, glm::mat4 value) const;
	void setMat3(const std::string& name, const glm::mat3& mat) const;

	GLint getUniformLocation(const std::string& name) const;
	
	void initUBOs();
	void bindUBOs();
	void setCameraData(const CameraData& cameraData);

	//Delete copy constructor and copy assignment
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	//Move constructor and move assignment
	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;

private:
	mutable std::unordered_map<std::string, GLint> uniformLocationCache;

	void checkCompileErrors(unsigned int shader, std::string type);
	unsigned int compileShader(const char* source, GLenum shaderType);
	unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
	
};
