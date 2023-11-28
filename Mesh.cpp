#include "Mesh.h"
#include "OpenGLUtils.h"

enum class VertexAttribute
{
	Position = 0,
	Normal,
	TexCoords,
	Tangent,
	Bitangent,
	
	MaxAttributes 
};

TextureStateManager texState;

void TextureStateManager::bind2D(GLuint tex)
{
	if (tex != currentlyBound2D)
	{
		if (tex != 0 && glIsTexture(tex) == GL_FALSE)
		{
			std::cerr << "Error: Texture ID " << tex << " is not a valid texture object." << std::endl;
			// Handle the error accordingly
		}
		glBindTexture(GL_TEXTURE_2D, tex);
		CHECK_GL_ERROR("glBindTexture(GL_TEXTURE_2D, tex)");
		currentlyBound2D = tex;
	}
}

void TextureStateManager::bindCubeMap(GLuint tex)
{
	if (tex != currentlyBoundCubeMap)
	{
		if (tex != 0 && glIsTexture(tex) == GL_FALSE)
		{
			std::cerr << "Error: Texture ID " << tex << " is not a valid texture object." << std::endl;
			// Handle the error accordingly
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
		//CHECK_GL_ERROR("glBindTexture(GL_TEXTURE_CUBE_MAP, tex)");
		currentlyBoundCubeMap = tex;
	}
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, 
	std::vector<unsigned int>& albedoMapIds,
	std::vector<unsigned int>& normalMapIds,
	std::vector<unsigned int>& roughnessMetallicMapIds,
	std::vector<unsigned int>& aoMapIds)
	: vertices(vertices), indices(indices), textureIDs(textureIDs),
	albedoMapIDs(albedoMapIds),
	normalMapIDs(normalMapIds),
	roughnessMetallicMapIDs(roughnessMetallicMapIds),
	aoMapIDs(aoMapIds)
{
	setupMesh();
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}


void Mesh::DrawPBR(const Shader& shader, const IBLTextures& iblTextures) const
{
	const int DIFFUSE = GL_TEXTURE0;
	const int IRRADIANCE = DIFFUSE + albedoMapIDs.size() + normalMapIDs.size() + roughnessMetallicMapIDs.size() + aoMapIDs.size();
	const int PREFILTER = IRRADIANCE + 1;
	const int BRDF_LUT = PREFILTER + 1;

	//bind maps
	bindTextures(shader, albedoMapIDs, TextureUnit::Diffuse, "material.albedoMap");
	bindTextures(shader, normalMapIDs, TextureUnit::Normal, "material.normalMap");
	bindTextures(shader, roughnessMetallicMapIDs, TextureUnit::RoughnessMetallic, "material.roughnessMetallicMap");
	bindTextures(shader, aoMapIDs, TextureUnit::AmbientOcclusion, "material.aoMap");

	// Print out texture units to verify correctness
	//std::cout << "Diffuse Texture Unit: " << DIFFUSE - GL_TEXTURE0 << std::endl;
	//std::cout << "Irradiance Texture Unit: " << IRRADIANCE - GL_TEXTURE0 << std::endl;
	//std::cout << "Prefilter Texture Unit: " << PREFILTER - GL_TEXTURE0 << std::endl;
	//std::cout << "BRDF LUT Texture Unit: " << BRDF_LUT - GL_TEXTURE0 << std::endl;

	//bind IBLs
	bindIBLTextures(shader, iblTextures, IRRADIANCE, PREFILTER, BRDF_LUT);

	if (glIsVertexArray(VAO) == GL_FALSE)
	{
		std::cerr << "Error: VAO ID " << VAO << " is not a valid vertex array object." << std::endl;
	}

	// Draw mesh
	glBindVertexArray(VAO);

	// Check if the VAO has an EBO bound to it
	GLint eboBound;
	glGetVertexArrayiv(VAO, GL_ELEMENT_ARRAY_BUFFER_BINDING, &eboBound);
	if (eboBound == 0)
	{
		std::cerr << "No EBO bound to VAO." << std::endl;
	}

	CHECK_GL_ERROR("glBindVertexArray(VAO)");
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	CHECK_GL_ERROR("glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0)");
	glBindVertexArray(0);
	CHECK_GL_ERROR("glBindVertexArray(0)");

	// Reset to default
	resetTextures(DIFFUSE, GL_TEXTURE0, IRRADIANCE, PREFILTER);

}
void Mesh::bindTextures(const Shader& shader, const std::vector<unsigned int>& textureIDs, TextureUnit textureUnit, const std::string& uniformName) const
{
	for (const auto& textureID : textureIDs)
	{
		if (glIsTexture(textureID) == GL_FALSE)
		{
			std::cerr << "Warning: Texture ID " << textureID << " is not valid." << std::endl;
			continue;
		}
		int unit = static_cast<int>(textureUnit);
		glActiveTexture(GL_TEXTURE0 + unit);
		shader.setInt(uniformName, unit);
		texState.bind2D(textureID);
		textureUnit = static_cast<TextureUnit>(unit + 1); // Move to the next texture unit for next texture
	}
}

void Mesh::bindIBLTextures(const Shader& shader, const IBLTextures& iblTextures, int irradianceUnit, int prefilterUnit, int brdfLUTUnit) const
{
	glActiveTexture(irradianceUnit);
	shader.setInt("irradianceMap", irradianceUnit - GL_TEXTURE0);
	texState.bindCubeMap(iblTextures.irradianceMap);

	glActiveTexture(prefilterUnit);
	shader.setInt("prefilterMap", prefilterUnit - GL_TEXTURE0);
	texState.bindCubeMap(iblTextures.prefilterMap);

	glActiveTexture(brdfLUTUnit);
	shader.setInt("brdfLUT", brdfLUTUnit - GL_TEXTURE0);
	texState.bind2D(iblTextures.brdfLUTTexture);
}

void Mesh::resetTextures(int startUnit, int endUnit, int irradianceUnit, int prefilterUnit) const
{
	glActiveTexture(startUnit);
	for (int unit = startUnit; unit < endUnit; ++unit)
	{
		glActiveTexture(unit);
		texState.bind2D(0);
	}

	glActiveTexture(irradianceUnit);
	texState.bindCubeMap(0);

	glActiveTexture(prefilterUnit);
	texState.bindCubeMap(0);
}

void Mesh::setupMesh()
{
	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// vertex attribute pointers setup
	GLsizei stride = sizeof(Vertex); // distance between each vertex in the vertex buffer in bytes

	glEnableVertexAttribArray(static_cast<GLint>(VertexAttribute::Position));
	glVertexAttribPointer(static_cast<GLint>(VertexAttribute::Position), 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(static_cast<GLint>(VertexAttribute::Normal));
	glVertexAttribPointer(static_cast<GLint>(VertexAttribute::Normal), 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Normal));

	glEnableVertexAttribArray(static_cast<GLint>(VertexAttribute::TexCoords));
	glVertexAttribPointer(static_cast<GLint>(VertexAttribute::TexCoords), 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, TexCoords));

	glEnableVertexAttribArray(static_cast<GLint>(VertexAttribute::Tangent));
	glVertexAttribPointer(static_cast<GLint>(VertexAttribute::Tangent), 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Tangent));

	glEnableVertexAttribArray(static_cast<GLint>(VertexAttribute::Bitangent));
	glVertexAttribPointer(static_cast<GLint>(VertexAttribute::Bitangent), 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, Bitangent));

	glBindVertexArray(0);
}

Mesh::Mesh(Mesh&& other) noexcept
	: VAO(std::exchange(other.VAO, 0)),
	VBO(std::exchange(other.VBO, 0)),
	EBO(std::exchange(other.EBO, 0)),
	vertices(std::move(other.vertices)),
	indices(std::move(other.indices)),
	albedoMapIDs(std::move(other.albedoMapIDs)),
	normalMapIDs(std::move(other.normalMapIDs)),
	roughnessMetallicMapIDs(std::move(other.roughnessMetallicMapIDs)),
	aoMapIDs(std::move(other.aoMapIDs)),
	textureIDs(std::move(other.textureIDs))
{
	//std::cout << "MESH WAS MOVED" << std::endl;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
	if (this != &other)
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);

		VAO = std::exchange(other.VAO, 0);
		VBO = std::exchange(other.VBO, 0);
		EBO = std::exchange(other.EBO, 0);
		vertices = std::move(other.vertices);
		indices = std::move(other.indices);
		albedoMapIDs = std::move(other.albedoMapIDs);
		normalMapIDs = std::move(other.normalMapIDs);
		roughnessMetallicMapIDs = std::move(other.roughnessMetallicMapIDs);
		aoMapIDs = std::move(other.aoMapIDs);
		textureIDs = std::move(other.textureIDs);
	}
	return *this;
}