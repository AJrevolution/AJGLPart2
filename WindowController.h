#pragma once

#include <glad/glad.h>
#include <glfw3.h>
#include "Camera.h"
#include "IWindowSizeChangeObserver.h"

class WindowController
{
std::vector<IWindowSizeChangeObserver*> sizeChangeObservers;

public:
    WindowController(GLFWwindow* window, Camera& camera, float screenWidth, float screenHeight);

    void processInput(float deltaTime);

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    void addWindowSizeChangeObserver(IWindowSizeChangeObserver* observer);

private:
    GLFWwindow* window;
    Camera& camera;

    static float lastX;
    static float lastY;
    static bool firstMouse;

    static WindowController* getController(GLFWwindow* window);

};

