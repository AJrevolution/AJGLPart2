#include "PBRTexture.h"
#include "stb_image.h"
#include <iostream>

//this class exists to streamline the PBRHelper to setup IBLs easier
PBRTexture::PBRTexture(const std::string& path, bool isHDR) : textureID(0), isHDR(isHDR)
{
    if(isHDR)
        loadTexture(path, isHDR);
    std::cout << "PBRTexture created at path: " << path << std::endl;
}

PBRTexture::~PBRTexture()
{
    std::cout << "PBRTexture destroyed" << std::endl;
    glDeleteTextures(1, &textureID);
}

void PBRTexture::bind(GLenum textureUnit) const
{
    glActiveTexture(textureUnit);
    glBindTexture(isHDR ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP, textureID);
}

unsigned int PBRTexture::getID() const
{
    return textureID;
}

void PBRTexture::loadTexture(const std::string& path, bool isHDR)
{
    glGenTextures(1, &textureID);
    glBindTexture(isHDR ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP, textureID);

    if (isHDR)
    {
        stbi_set_flip_vertically_on_load(true);
        int width, height, nrComponents;
        float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format = nrComponents == 3 ? GL_RGB16F : GL_RGBA16F;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_FLOAT, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Failed to load HDR texture: " << path << std::endl;
        }
    }
 
}

void PBRTexture::createBRDFLUTTexture(GLsizei width, GLsizei height)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, nullptr);

    // Set texture parameters
    setupTextureParameters(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

    //glBindTexture(GL_TEXTURE_2D, 0);
}

void PBRTexture::setupTextureParameters(GLenum wrapS, GLenum wrapT, GLenum minFilter, GLenum magFilter)
{
    //glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    //glBindTexture(GL_TEXTURE_2D, 0);
}

