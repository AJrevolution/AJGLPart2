#pragma once
#include <glad/glad.h>

class RenderHelper
{
public:
    static void init();
    static void cleanup();

    static void renderCube();
    static void renderQuad();

    static void setupCube();
    static void setupQuad();

private:
    static unsigned int cubeVAO, cubeVBO;
    static unsigned int quadVAO, quadVBO;

   
};

