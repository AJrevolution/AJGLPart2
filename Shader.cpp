#include "Shader.h"
#include <vector>


Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex = compileShader(vShaderCode, GL_VERTEX_SHADER);
	unsigned int fragment = compileShader(fShaderCode, GL_FRAGMENT_SHADER);

	ID = linkProgram(vertex, fragment);

	if (glIsProgram(ID) == GL_FALSE)
	{
		std::cerr << "Error: Shader program ID " << ID << " is not a valid program object." << std::endl;
	}

	//check for errors
	GLint success;
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		std::cerr << "Error: Shader program ID " << ID << " IT WAS NOT A SUCCESS" << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	std::cout << "Shader created: " << std::endl;
}

Shader::~Shader()
{
	if (ID != 0)
	{
		glDeleteProgram(ID);
	}

}

//RAII principles

//Move constructor
Shader::Shader(Shader&& other) noexcept
	: ID(std::exchange(other.ID, 0)),  // Nullify the source object to prevent double deletion
	uboCamera(std::exchange(other.uboCamera, 0)), 
	uboGlobal(std::exchange(other.uboGlobal, 0)),
	uniformLocationCache(std::move(other.uniformLocationCache)) 
{
	std::cout << "SHADER WAS MOVED" << std::endl;
}

// Move assignment
Shader& Shader::operator=(Shader&& other) noexcept
{
	if (this != &other)
	{
		glDeleteProgram(ID);
		ID = std::exchange(other.ID, 0); // Transfer ownership and nullify the source
		uboCamera = std::exchange(other.uboCamera, 0); // Transfer UBOs
		uboGlobal = std::exchange(other.uboGlobal, 0);
		uniformLocationCache = std::move(other.uniformLocationCache); // Move the cache
	}

	return *this;
}

void Shader::use()
{
	glUseProgram(ID);
}

//Utility uniform functions
void Shader::setBool(const std::string& name, bool value) const
{
	GLint location = getUniformLocation(name);
	glUniform1i(location, (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
	GLint location = getUniformLocation(name);
	glUniform1i(location, (int)value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	GLint location = getUniformLocation(name);
	glUniform1f(location, value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	GLint location = getUniformLocation(name);
	glUniform3fv(location, 1, &value[0]);
}

void Shader::setMat4(const std::string& name, glm::mat4 value) const
{
    GLint location = getUniformLocation(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
	GLint location = getUniformLocation(name);
	glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
}


void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
	int success;

    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            int length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> infoLog(length);
            glGetShaderInfoLog(shader, length, NULL, &infoLog[0]);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << &infoLog[0] << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            int length;
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> infoLog(length);
            glGetProgramInfoLog(shader, length, NULL, &infoLog[0]);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << &infoLog[0] << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

unsigned int Shader::compileShader(const char* source, GLenum shaderType)
{
	unsigned int shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	checkCompileErrors(shader, shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
	return shader;
}

unsigned int Shader::linkProgram(unsigned int vertexShader, unsigned int fragmentShader)
{
	unsigned int program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	checkCompileErrors(program, "PROGRAM");
	return program;
}

GLint Shader::getUniformLocation(const std::string& name) const
{
	//Check if location is cached
	if (uniformLocationCache.find(name) != uniformLocationCache.end())
	{
		return uniformLocationCache[name];
	}

	//If not get the location and cache it
	GLint location = glGetUniformLocation(ID, name.c_str());
	if (location == -1)
	{
		std::cerr << "Warning: Uniform '" << name << "' doesn't exist in shader!" << std::endl;
	}
	std::cout << "shader uniform location: " << location << std::endl;
	uniformLocationCache[name] = location;
	return location;
}

//std::string TextureTypeToString(TextureType type)
//{
//	switch (type)
//	{
//	case TextureType::DIFFUSE: return "texture_diffuse";
//	case TextureType::SPECULAR: return "texture_specular";
//	case TextureType::NORMAL: return "texture_normal";
//	case TextureType::HEIGHT: return "texture_height";
//	default: return "";
//	}
//}

void Shader::initUBOs()
{
	glGenBuffers(1, &uboCamera);
	glBindBuffer(GL_UNIFORM_BUFFER, uboCamera);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2 + sizeof(glm::vec3), NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &uboGlobal);
	glBindBuffer(GL_UNIFORM_BUFFER, uboGlobal);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) + sizeof(float), NULL, GL_STATIC_DRAW);
}

void Shader::bindUBOs()
{
	GLuint bindingPointCamera = 0;
	GLuint blockIndexCamera = glGetUniformBlockIndex(ID, "CameraMatrices");
	glUniformBlockBinding(ID, blockIndexCamera, bindingPointCamera);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPointCamera, uboCamera);
}

void Shader::setCameraData(const CameraData& cameraData)
{
	glBindBuffer(GL_UNIFORM_BUFFER, uboCamera);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(cameraData.viewMatrix));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(cameraData.projectionMatrix));
	glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::vec3), glm::value_ptr(cameraData.camPos));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}