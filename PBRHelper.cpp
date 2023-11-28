#include "PBRHelper.h"
#include <glm/gtc/matrix_transform.hpp>
#include "OpenGLUtils.h"

PBRHelper::PBRHelper(std::shared_ptr<ResourceManager> rm)
    : resourceManager(std::move(rm)),
    envCubeMap(512, GL_RGB16F, GL_LINEAR_MIPMAP_LINEAR),
    irradianceMap(32, GL_RGB16F),
    prefilterMap(128, GL_RGB16F, GL_LINEAR_MIPMAP_LINEAR, true),
    brdfLUTTexture(),
    equirectangularToCubemapShader("ShaderFiles\\cubemap.vs.txt", "ShaderFiles\\cubemap.fs.txt"),
    irradianceShader("ShaderFiles\\backgroundHDR.vs.txt", "ShaderFiles\\irradiance.fs.txt"),
    preFilterShader("ShaderFiles\\backgroundHDR.vs.txt", "ShaderFiles\\preFilter.fs.txt"),
    BRDFShader("ShaderFiles\\brdf.vs.txt", "ShaderFiles\\brdf.fs.txt"),
    backgroundHDRShader("ShaderFiles\\backgroundHDR.vs.txt", "ShaderFiles\\backgroundHDR.fs.txt")
{
    std::cout << "PBRHelper created" << std::endl;
    // Initialize capture views for cubemap
    initialiseCaptureViews();
    initialiseRenderBuffer(512, 512);

    // Initialize Framebuffer and RenderHelper
    captureFBO = std::make_unique<Framebuffer>();
    glGenRenderbuffers(1, &captureRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512); // Adjust size as needed
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    renderHelper = RenderHelper();
}

PBRHelper::~PBRHelper()
{
    std::cout << "PBRHelper destroyed" << std::endl;
    // Destructor code if needed
}

void PBRHelper::initialiseCaptureViews()
{
    captureViews[0] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    captureViews[1] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    captureViews[2] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    captureViews[3] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    captureViews[4] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    captureViews[5] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
}

void PBRHelper::initialiseRenderBuffer(GLsizei width, GLsizei height)
{
    glGenRenderbuffers(1, &captureRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void PBRHelper::SetupEnvironment(const std::string& hdrPath)
{
    loadHDR(hdrPath);
    convertEquirectangularToCubemap(hdrPath);
    generateIrradianceMap();
    generatePrefilterMap();
    generateBRDFLUT();
}

void PBRHelper::SetupIrradianceMap()
{
    generateIrradianceMap();
}

void PBRHelper::SetupPrefilterMap()
{
    generatePrefilterMap();
}

void PBRHelper::SetupBRDFLUT()
{
    generateBRDFLUT();
}

void PBRHelper::loadHDR(const std::string& path)
{
    Texture hdrTexture = resourceManager->getTexture(path, "", aiTextureType_UNKNOWN, true);
    //brdfLUTTextureID = hdrTexture.id;
}

void PBRHelper::renderEnvironment(const glm::mat4& viewMatrix, const glm::mat4& projection, GLuint textureID)
{
    backgroundHDRShader.use();
    backgroundHDRShader.setMat4("view", viewMatrix);
    backgroundHDRShader.setMat4("projection", projection);
    backgroundHDRShader.setInt("environmentMap", 0);
    //envCubeMap.bind(GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    renderHelper.renderCube(); 
}

void PBRHelper::convertEquirectangularToCubemap(const std::string& hdrPath)
{
    //int currentFBO;
    //glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    //std::cout << "Current bound FBO before binding captureFBO: " << currentFBO << std::endl;

    // Temporary HDR texture to load the equirectangular image
    PBRTexture hdrTexture(hdrPath, true); // Load HDR texture
    equirectangularToCubemapShader.use();
    equirectangularToCubemapShader.setInt("equirectangularMap", 0); // Set the uniform
    equirectangularToCubemapShader.setMat4("projection", glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
    hdrTexture.bind(GL_TEXTURE0);

    glViewport(0, 0, 512, 512);

    captureFBO->bind();
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    //CHECK_GL_ERROR("glBindTexture(GL_TEXTURE_2D, tex)");

    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader.setMat4("view", captureViews[i]);
        captureFBO->attachTexture(envCubeMap.getID(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderHelper.renderCube();
    }
    captureFBO->unbind();

    envCubeMap.generateMipmaps();
}

void PBRHelper::generateIrradianceMap()
{
    irradianceShader.use();
    irradianceShader.setInt("environmentMap", 0);
    irradianceShader.setMat4("projection", glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
    envCubeMap.bind(GL_TEXTURE0);
    glViewport(0, 0, 32, 32);
    captureFBO->bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader.setMat4("view", captureViews[i]);
        captureFBO->attachTexture(irradianceMap.getID(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderHelper.renderCube();
    }
    captureFBO->unbind();

    irradianceMap.generateMipmaps(); 


    std::cout << "Irradiance Map Texture ID: " << irradianceMap.getID() << std::endl;
    
    iblTextures.irradianceMap = irradianceMap.getID();

}

void PBRHelper::generatePrefilterMap()
{
    preFilterShader.use();
    preFilterShader.setInt("environmentMap", 0);
    preFilterShader.setMat4("projection", glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
    envCubeMap.bind(GL_TEXTURE0);

    captureFBO->bind(); // Bind the FBO here
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        unsigned int mipWidth = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = mipWidth;
        //captureFBO->resize(mipWidth, mipHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        preFilterShader.setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            preFilterShader.setMat4("view", captureViews[i]);
            captureFBO->attachTexture(prefilterMap.getID(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderHelper.renderCube();
        }
    }
    captureFBO->unbind(); // Unbind the FBO here

    std::cout << "Prefilter Map Texture ID: " << prefilterMap.getID() << std::endl;

    iblTextures.prefilterMap = prefilterMap.getID();
}

void PBRHelper::generateBRDFLUT()
{
    BRDFShader.use();

    // Create and configure the BRDF LUT texture
    brdfLUTTexture.createBRDFLUTTexture(512, 512); 

    captureFBO->bind();
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    captureFBO->attachTexture(brdfLUTTexture.getID(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0);
    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderHelper.renderQuad(); // Render a quad to generate the BRDF LUT
    captureFBO->unbind();

    // Store the BRDF LUT texture ID in IBLTextures
    iblTextures.brdfLUTTexture = brdfLUTTexture.getID();
}

const IBLTextures& PBRHelper::getIBLTextures() const
{
    return iblTextures;
}

const GLuint PBRHelper::getEnvironmentMapID() const
{
    return envCubeMap.getID();
}

const GLuint PBRHelper::getIrradianceMapID() const
{
    return irradianceMap.getID();
}

const GLuint PBRHelper::getPrefilterMapID() const
{
    return prefilterMap.getID();
}

const GLuint PBRHelper::getBRDFLUTTextureID() const
{
    return brdfLUTTexture.getID();
}