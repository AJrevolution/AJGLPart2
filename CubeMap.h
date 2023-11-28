#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class CubeMap
{
public:
    CubeMap(GLsizei size, GLenum internalFormat = GL_RGB16F, GLenum minFilterParameter = GL_LINEAR, bool generateMipsImmediately = false);
    ~CubeMap();

    CubeMap(const CubeMap&) = delete;
    CubeMap& operator=(const CubeMap&) = delete;

    CubeMap(CubeMap&& other) noexcept;
    CubeMap& operator=(CubeMap&& other) noexcept;

    void bind(GLenum textureUnit) const;
    void attachToFramebuffer(GLenum attachment, GLenum face, GLint level = 0) const;
    void setWrapParameters(GLenum s, GLenum t, GLenum r);
    void setFilterParameters(GLenum minFilter, GLenum magFilter);
    void generateMipmaps();

    GLuint getID() const { return ID; }

private:
    GLuint ID;
    GLsizei size;
};
