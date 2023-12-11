#pragma once
#include <glad/glad.h>
#include "IWindowSizeChangeObserver.h"
#include "Framebuffer.h"
#include <memory>

class SuperSamplingRenderer : public IWindowSizeChangeObserver
{
public:
    SuperSamplingRenderer(unsigned int screenWidth, unsigned int screenHeight, unsigned int scale);
    ~SuperSamplingRenderer();

    void beginRender();
    void endRender();

    void updateViewportDimensions(int width, int height);

    void onWindowSizeChanged(int width, int height) override;

private:
    //std::unique_ptr<Framebuffer> ssFramebuffer; //removed cuz it broke everything, something about opengl context being bad everytime this class in particular uses FrameBuffer class
    GLuint ssFBO, ssColorBuffer, depthBuffer;
    GLint originalViewport[4];  // Array to store the original viewport dimensions during different glViewport calls
    unsigned int screenWidth, screenHeight, scale;
    
    void init();
    void cleanup();
};

