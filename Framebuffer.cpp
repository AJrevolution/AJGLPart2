#include "Framebuffer.h"
#include <iostream>
#include "OpenGLUtils.h"

Framebuffer::Framebuffer()
{
    glGenFramebuffers(1, &fbo);
    CHECK_GL_ERROR("glGenFramebuffers(1, &fbo)");
    std::cout << "fbo value in framebuffer: " << fbo << std::endl;
}

Framebuffer::~Framebuffer()
{
    std::cout << "fbo value : " << fbo << " destroyed" << std::endl;
    glDeleteFramebuffers(1, &fbo);
}


Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : fbo(std::exchange(other.fbo, 0)) 
{
    std::cout << "FrameBuffer moved" << std::endl;
}


Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other)
    {
        glDeleteFramebuffers(1, &fbo); // Clean up existing framebuffer
        fbo = std::exchange(other.fbo, 0); // Transfer ownership
    }
    std::cout << "FrameBuffer operator=" << std::endl;

    return *this;
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    CHECK_GL_ERROR("glBindFramebuffer(GL_FRAMEBUFFER, fbo);");
}

void Framebuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERROR("glBindFramebuffer(GL_FRAMEBUFFER, 0);");
}

void Framebuffer::attachTexture(GLuint textureID, GLenum attachmentType, GLenum texTarget, GLint mipLevel) const
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, texTarget, textureID, mipLevel);
    CHECK_GL_ERROR("glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, texTarget, textureID, mipLevel);");
}

void Framebuffer::attachRenderBuffer(GLuint renderBufferID, GLenum attachmentType) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, renderBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::checkStatus() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    bool status = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return status;
}

void Framebuffer::resize(GLsizei width, GLsizei height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Resize cubemap textures if attached
    for (GLuint i = 0; i < 6; ++i)
    {
        GLuint texId;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, reinterpret_cast<GLint*>(&texId));
        if (texId != 0) // Check if texture exists
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, texId);
            for (GLuint face = 0; face < 6; ++face)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
            }
        }
    }

    // Resize depth renderbuffer if attached
    GLuint rboId;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, reinterpret_cast<GLint*>(&rboId));
    if (rboId != 0) // Check if renderbuffer exists
    {
        glBindRenderbuffer(GL_RENDERBUFFER, rboId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}