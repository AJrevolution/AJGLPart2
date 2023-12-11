#include "SuperSamplingRenderer.h"
#include <iostream>

SuperSamplingRenderer::SuperSamplingRenderer(unsigned int screenWidth, unsigned int screenHeight, unsigned int scale)
    : screenWidth(screenWidth), screenHeight(screenHeight), scale(scale), ssFBO(0), ssColorBuffer(0), depthBuffer(0)
{
    init();
}

SuperSamplingRenderer::~SuperSamplingRenderer()
{
    cleanup();
}

void SuperSamplingRenderer::init()
{
    glGenFramebuffers(1, &ssFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssFBO);

    glGenTextures(1, &ssColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth * scale, screenHeight * scale, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssColorBuffer, 0);

    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth * scale, screenHeight * scale);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);



    //Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Supersampling framebuffer is not complete!" << std::endl;

    //Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Clean up bindings
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SuperSamplingRenderer::cleanup()
{
    glDeleteRenderbuffers(1, &depthBuffer);
    glDeleteTextures(1, &ssColorBuffer);
    glDeleteFramebuffers(1, &ssFBO);
}

void SuperSamplingRenderer::beginRender()
{
    glGetIntegerv(GL_VIEWPORT, originalViewport); // Save the current viewport
    glBindFramebuffer(GL_FRAMEBUFFER, ssFBO);
    glViewport(0, 0, screenWidth * scale, screenHeight * scale);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
}

void SuperSamplingRenderer::endRender()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, ssFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Default framebuffer
    glBlitFramebuffer(0, 0, screenWidth * scale, screenHeight * scale, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]); // Restore the original viewport
}

void SuperSamplingRenderer::updateViewportDimensions(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
}

void SuperSamplingRenderer::onWindowSizeChanged(int width, int height)
{
    updateViewportDimensions(width, height);
}