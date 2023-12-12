#include "ResourceManager.h"


#include <iostream>
#include "Shader.h"
#include "OpenGLUtils.h"

GLfloat ResourceManager::maxAnisotropy = 0.0f;

Texture ResourceManager::getTexture(const std::string& path, const std::string& directory,aiTextureType type, bool isHDR)
{
    // Check if the texture is already loaded
    auto it = textures.find(path);
    if (it != textures.end())
    {
        return it->second;
    }

    // Load the texture
    Texture newTexture = loadTextureFromFile(path, directory,aiTextureTypeToTextureType(type), isHDR);
    textures.insert({ path, newTexture });
    return textures[path];
}


Texture ResourceManager::loadTextureFromFile(const std::string& path, const std::string& directory, TextureType type, bool isHDR)
{

    std::string filename = directory + '/' + path;

    unsigned int textureID;
    glGenTextures(1, &textureID);


    int width, height, nrComponents;

    glBindTexture(GL_TEXTURE_2D, textureID);

    if (isHDR)
    {
        // Load HDR texture
        float* hdrData = stbi_loadf(filename.c_str(), &width, &height, &nrComponents, 0);
        if (hdrData)
        {
            GLenum format = (nrComponents == 3) ? GL_RGB16F : GL_RGBA16F;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_FLOAT, hdrData);
            stbi_image_free(hdrData);
        }
    }
    else
    {
        // Load non-HDR texture
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }
    // Check if the texture was loaded successfully
    if (glIsTexture(textureID) == GL_FALSE)
    {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        std::cout << "STBI Error: " << stbi_failure_reason() << std::endl;
        return Texture{}; // Return an empty Texture object
    }

    // Generate Mipmaps and set texture parameters
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Apply anisotropic filtering if supported
    if (maxAnisotropy > 0.0f)
    {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }

    Texture texture;
    texture.id = textureID;
    texture.type = type;
    texture.path = path;
    texture.isHDR = isHDR;
    return texture;
}


unsigned int ResourceManager::CreateDefaultTexture(unsigned char r, unsigned char g, unsigned char b)
{
    unsigned int textureID;
    unsigned char data[] = { r, g, b };

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

    return textureID;
}

void ResourceManager::InitMaxAnisotropy()
{
    if (GLAD_GL_EXT_texture_filter_anisotropic)
    {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    }
}

TextureType ResourceManager::aiTextureTypeToTextureType(aiTextureType type)
{
    switch (type)
    {
    case aiTextureType_DIFFUSE:
        return TextureType::DIFFUSE;
    case aiTextureType_SPECULAR:
        return TextureType::SPECULAR;
    case aiTextureType_HEIGHT:
        return TextureType::HEIGHT;
    case aiTextureType_NORMALS:
        return TextureType::NORMAL;
    case aiTextureType_SHININESS:
        return TextureType::SHININESS;
    case aiTextureType_AMBIENT:
        return TextureType::AMBIENT;
    case aiTextureType_AMBIENT_OCCLUSION:
        return TextureType::AMBIENT_OCCLUSION;
    case aiTextureType_BASE_COLOR:
        return TextureType::BASE_COLOR;
    case aiTextureType_UNKNOWN:
    default:
        return TextureType::UNKNOWN;
    }
}


std::string ResourceManager::TextureTypeToString(TextureType type)
{
    switch (type)
    {
    case TextureType::DIFFUSE: return "material.albedoMap";
    case TextureType::SPECULAR: return "material.texture_specular";
    case TextureType::NORMAL: return "material.normalMap";
    case TextureType::HEIGHT: return "material.normalMap";
    case TextureType::SHININESS: return "material.roughnessMetallicMap";
    case TextureType::AMBIENT: return "material.aoMap";
    case TextureType::AMBIENT_OCCLUSION: return "material.aoMap";
    case TextureType::BASE_COLOR: return "material.albedoMap";
    case TextureType::UNKNOWN: return "material.roughnessMetallicMap";
    default: return "";
    }
}
