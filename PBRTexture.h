#pragma once
#include <glad/glad.h>
#include <string>

class PBRTexture
{
public:
    PBRTexture() = default;
    explicit PBRTexture(const std::string& path, bool isHDR = false);
    ~PBRTexture();

    void bind(GLenum textureUnit) const;
    unsigned int getID() const;

    void createBRDFLUTTexture(GLsizei width, GLsizei height);

private:
    unsigned int textureID;
    bool isHDR;

    void loadTexture(const std::string& path, bool isHDR);
   
    void setupTextureParameters(GLenum wrapS, GLenum wrapT, GLenum minFilter, GLenum magFilter);
};
