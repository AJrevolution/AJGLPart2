#pragma once

#include "ResourceManager.h"
#include "Shader.h"
#include "RenderHelper.h"
#include "Framebuffer.h"
#include "CubeMap.h"
#include <glm/glm.hpp>
#include <string>
#include "PBRTexture.h"

struct IBLTextures
{
    GLuint irradianceMap = 0;
    GLuint prefilterMap = 0;
    GLuint brdfLUTTexture = 0;
};

//this class exists to streamline the PBRHelper to setup IBLs easier
class PBRHelper
{
public:
    PBRHelper(std::shared_ptr<ResourceManager> resourceManager);
    ~PBRHelper();

    void SetupEnvironment(const std::string& hdrPath);
    void SetupIrradianceMap();
    void SetupPrefilterMap();
    void SetupBRDFLUT();

    // Accessors for cube maps and textures
    const GLuint getEnvironmentMapID() const;
    const GLuint getIrradianceMapID() const;
    const GLuint getPrefilterMapID() const;
    const GLuint getBRDFLUTTextureID() const;

    const IBLTextures& getIBLTextures() const;
    void renderEnvironment(const glm::mat4& viewMatrix, const glm::mat4& projection, GLuint textureID);
    void convertEquirectangularToCubemap(const std::string& hdrPath);
    void generateIrradianceMap();
    void generatePrefilterMap();
    void generateBRDFLUT();
private:
    std::shared_ptr<ResourceManager> resourceManager;

    CubeMap envCubeMap;
    CubeMap irradianceMap;
    CubeMap prefilterMap;
    PBRTexture brdfLUTTexture;

    Shader equirectangularToCubemapShader;
    Shader irradianceShader;
    Shader preFilterShader;
    Shader BRDFShader;
    Shader backgroundHDRShader;

    std::unique_ptr<Framebuffer> captureFBO;
    GLuint captureRBO;

    RenderHelper renderHelper;

    glm::mat4 captureViews[6]; // Capture views for cubemap generation

    IBLTextures iblTextures;

    void loadHDR(const std::string& path);
    
    void initialiseCaptureViews();
    void initialiseRenderBuffer(GLsizei width, GLsizei height);

    //Viewport management for SSAA
    GLint savedViewport[4];
    void saveViewport();
    void restoreViewport();
};
