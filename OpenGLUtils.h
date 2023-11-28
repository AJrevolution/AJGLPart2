#pragma once

#include <glad/glad.h>
#include <iostream>

namespace OpenGLUtils
{
    void CheckOpenGLError(const char* statement, const char* filename, int line);
    void APIENTRY openglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

#define CHECK_GL_ERROR(stmt) OpenGLUtils::CheckOpenGLError(stmt, __FILE__, __LINE__)
#define SETUP_OPENGL_DEBUG_CALLBACK() \
    do { \
        glEnable(GL_DEBUG_OUTPUT); \
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); \
        glDebugMessageCallback(OpenGLUtils::openglDebugCallback, nullptr); \
    } while(0)
}
