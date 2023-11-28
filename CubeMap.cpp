#include "CubeMap.h"
#include <iostream>

CubeMap::CubeMap(GLsizei size, GLenum internalFormat, GLenum minFilterParameter, bool generateMipsImmediately) : size(size)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

    for (GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
            size, size, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    // Set default wrap and filter parameters
    setWrapParameters(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    setFilterParameters(minFilterParameter, GL_LINEAR);

    if (generateMipsImmediately && (minFilterParameter == GL_LINEAR_MIPMAP_LINEAR ||
        minFilterParameter == GL_NEAREST_MIPMAP_NEAREST ||
        minFilterParameter == GL_LINEAR_MIPMAP_NEAREST ||
        minFilterParameter == GL_NEAREST_MIPMAP_LINEAR))
    {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Unbind the texture

    std::cout << "CubeMap created" << std::endl;
}

CubeMap::~CubeMap()
{
    std::cout << "CubeMap destroyed" << std::endl;
    glDeleteTextures(1, &ID);
}

CubeMap::CubeMap(CubeMap&& other) noexcept : ID(other.ID), size(other.size)
{
    other.ID = 0; // Use the 'null' texture ID
    std::cout << "CubeMap moved &&" << std::endl;
}

CubeMap& CubeMap::operator=(CubeMap&& other) noexcept
{
    if (this != &other)
    {
        glDeleteTextures(1, &ID);
        ID = other.ID;
        size = other.size;
        other.ID = 0;
    }

    std::cout << "CubeMap operator=" << std::endl;
    return *this;
}

void CubeMap::bind(GLenum textureUnit) const
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}

void CubeMap::attachToFramebuffer(GLenum attachment, GLenum face, GLint level) const
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, face, ID, level);
}

void CubeMap::setWrapParameters(GLenum s, GLenum t, GLenum r)
{
    //glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, s);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, t);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, r);
}

void CubeMap::setFilterParameters(GLenum minFilter, GLenum magFilter)
{
    //glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);
}

void CubeMap::generateMipmaps()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

// Helper function to check framebuffer status
bool checkFramebufferStatus()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer not complete: " << status << std::endl;
        return false;
    }
    return true;
}
