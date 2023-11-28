#pragma once
#include <glad/glad.h>
#include <vector>

class Framebuffer
{
public:
    Framebuffer();
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    void bind() const;
    void unbind() const;
    void attachTexture(GLuint textureID, GLenum attachmentType, GLenum texTarget, GLint mipLevel = 0) const;
    void attachRenderBuffer(GLuint renderBufferID, GLenum attachmentType) const;
    bool checkStatus() const;

    // Additional utility functions
    void resize(GLsizei width, GLsizei height);
    GLuint getID() const { return fbo; }

private:
    GLuint fbo;
};

