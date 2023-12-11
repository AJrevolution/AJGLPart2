#include "WindowController.h"


float WindowController::lastX = 0;
float WindowController::lastY = 0;
bool WindowController::firstMouse = true;

WindowController::WindowController(GLFWwindow* window, Camera& camera, float screenWidth, float screenHeight)
    : window(window), camera(camera)
{
    lastX = screenWidth / 2.0f;
    lastY = screenHeight / 2.0f;

    glfwSetWindowUserPointer(window, this); 

    glfwSetFramebufferSizeCallback(window, WindowController::framebuffer_size_callback);
    glfwSetCursorPosCallback(window, WindowController::mouse_callback);
    glfwSetScrollCallback(window, WindowController::scroll_callback);
}


void WindowController::processInput(float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void WindowController::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    WindowController* controller = getController(window);
    for (auto* observer : controller->sizeChangeObservers)
    {
        observer->onWindowSizeChanged(width, height);
    }
}

void WindowController::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    WindowController* controller = getController(window);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    controller->camera.ProcessMouseMovement(xoffset, yoffset);
}

void WindowController::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    WindowController* controller = getController(window);
    controller->camera.ProcessMouseScroll(yoffset);
}

WindowController* WindowController::getController(GLFWwindow* window)
{
    return static_cast<WindowController*>(glfwGetWindowUserPointer(window));
}

void WindowController::addWindowSizeChangeObserver(IWindowSizeChangeObserver* observer)
{
    sizeChangeObservers.push_back(observer);
}