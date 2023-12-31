// AJGL.cpp : Defines the entry point for the console application.
//

#define STB_IMAGE_IMPLEMENTATION

#include "stdafx.h"

#include <iostream>
#include <glad/glad.h>
#include <glfw3.h>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include <filesystem>
#include "skyboxdata.h"
#include "OpenGLUtils.h"
#include "PBRHelper.h"
#include "WindowController.h"
#include "SuperSamplingRenderer.h"

//Screen Dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame


struct ShaderGroup
{
	Shader shader;
	std::vector<Model*> models;
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;
	float accumulatedTime = 0.0f;

	ShaderGroup(const char* vertexPath, const char* fragmentPath)
		: shader(vertexPath, fragmentPath)
	{
	}

	void updateLights(glm::mat4 viewMatrix, float deltaTime)
	{
		accumulatedTime += deltaTime;  // Accumulate time

		for (unsigned int i = 0; i < lightPositions.size(); ++i)
		{
			float timeFactor = accumulatedTime * (1.0 + 0.5 * i); // Adjust time factor
			glm::vec3 movement = glm::vec3(sin(timeFactor) * 3.0, sin(timeFactor * 0.75) * 1.5, sin(timeFactor * 0.5) * 2.0);
			glm::vec3 newPos = lightPositions[i] + movement;
			shader.setVec3("lights[" + std::to_string(i) + "].Position", newPos);
			shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
		}
	}
};

std::vector<ShaderGroup> shaderGroups;


int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//Create Camera instance
	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

	//Create WindowController instance
	WindowController windowController(window, camera, SCR_WIDTH, SCR_HEIGHT);

	//glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}



	// configure global opengl state
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize and configure the supersampling renderer
	SuperSamplingRenderer ssRenderer(SCR_WIDTH, SCR_HEIGHT, 2);
	windowController.addWindowSizeChangeObserver(&ssRenderer); // Register ssRenderer with windowController

	//DEBUG
	//SETUP_OPENGL_DEBUG_CALLBACK();


	//Prepare skybox for IBLs
	RenderHelper::init();

	//Compile PBR shader 
	ShaderGroup pbrGroup("ShaderFiles\\PBRShader.vc.txt"
		, "ShaderFiles\\PBRShader.fc.txt");

	//Handle textures for each model
	auto rManager = std::make_shared<ResourceManager>();

	// Initialize max anisotropy level
	ResourceManager::InitMaxAnisotropy();

	//Generate IBL Textures and CubeMaps
	PBRHelper pbrHelper(rManager);

	//Load model and add to PBR Group
	Model raceTrack("Models\\PBRObjects\\realracetrack", "Models\\PBRObjects\\realracetrack\\racetrackentire.gltf", rManager, true);
	raceTrack.setPosition(glm::vec3(0.0f, -2.75f, 0.0f));

	pbrGroup.models.push_back(&raceTrack);

	//Light setup
	pbrGroup.lightPositions =
	{
	glm::vec3(0.0f, 0.0f, 10.0f),
	glm::vec3(10.0f, 0.0f, 0.0f),
	glm::vec3(-10.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 10.0f, 0.0f),      // Overhead light
	glm::vec3(0.0f, -10.0f, 0.0f)     // Under light
	};

	pbrGroup.lightColors =
	{
		glm::vec3(80.0f, 160.0f, 160.0f),
		glm::vec3(120.0f, 100.0f, 170.0f),
		glm::vec3(200.0f, 140.0f, 100.0f),
		glm::vec3(60.0f, 60.0f, 80.0f),   // Dimmer overhead light color
		glm::vec3(80.0f, 90.0f, 180.0f)   // Slightly brighter under-light color for added contrast
	};

	pbrGroup.shader.use();
	pbrGroup.shader.initUBOs();
	pbrGroup.shader.bindUBOs();
	shaderGroups.push_back(std::move(pbrGroup));

	//Load HDR and setup IBLs for rendering
	std::string hdrPath = "Images\\skybox\\SPACE-2.hdr"; //lilienstein_2k
	pbrHelper.SetupEnvironment(hdrPath);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	int scrWidth, scrHeight;
	glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
	glViewport(0, 0, scrWidth, scrHeight);

	IBLTextures iblLighting = pbrHelper.getIBLTextures();		//this should be modified if objects ever have different IBLs per model TODO

	//DeltaTime calculation
	float currentFrame;
	glm::vec3 scale = glm::vec3(2.0f, 2.0f, 2.0f);

	//While window is active loop
	while (!glfwWindowShouldClose(window))
	{
		//Start supersampling rendering
		ssRenderer.beginRender();

		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Detect input
		windowController.processInput(deltaTime);

		//Rendering commands
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		

		//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		glm::mat4 view = camera.GetViewMatrix();

	

		//Rendering loop
		for (auto & shaderGroup : shaderGroups)
		{
			shaderGroup.shader.use();

			shaderGroup.updateLights(view, deltaTime);
		
			CameraData cameraData;
			cameraData.viewMatrix = view;
			cameraData.projectionMatrix = projection;
			cameraData.camPos = camera.Position;
			shaderGroup.shader.setCameraData(cameraData);
		
			shaderGroup.shader.setInt("material.albedoMap",				static_cast<int>(TextureUnit::Diffuse));
			shaderGroup.shader.setInt("material.normalMap",				static_cast<int>(TextureUnit::Normal));
			shaderGroup.shader.setInt("material.roughnessMetallicMap",	static_cast<int>(TextureUnit::RoughnessMetallic));
			shaderGroup.shader.setInt("material.aoMap",					static_cast<int>(TextureUnit::AmbientOcclusion));
			shaderGroup.shader.setInt("irradianceMap",					static_cast<int>(TextureUnit::Irradiance));
			shaderGroup.shader.setInt("prefilterMap",					static_cast<int>(TextureUnit::Prefilter));
			shaderGroup.shader.setInt("brdfLUT",						static_cast<int>(TextureUnit::BrdfLUT));
			
			//Render each model
			for (const auto& modelObject : shaderGroup.models)
			{				
				shaderGroup.shader.setMat4("model", modelObject->modelMatrix);
				glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelObject->modelMatrix)));
				shaderGroup.shader.setMat3("normalMatrix", normalMatrix);
				modelObject->DrawPBR(shaderGroup.shader, iblLighting);
			}
		}

		//pbrHelper.generateIrradianceMap(); //debug
		//Render skybox and IBLs
		pbrHelper.renderEnvironment(view, projection, pbrHelper.getEnvironmentMapID());

		//End supersampling rendering and downscale the image to screen size
		ssRenderer.endRender();

		//Check and call events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
