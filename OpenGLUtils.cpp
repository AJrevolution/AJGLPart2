#include "OpenGLUtils.h"

namespace OpenGLUtils
{
    void CheckOpenGLError(const char* statement, const char* filename, int line)
    {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            std::cerr << "OpenGL error " << err << " at " << filename << ":" << line << " - for " << statement << std::endl;
            // Handle or throw error as appropriate for your application

            switch (err)
            {
            case GL_NO_ERROR:
                std::cerr << "No error" << std::endl;
            case GL_INVALID_ENUM:
                std::cerr << "Invalid enum" << std::endl;
            case GL_INVALID_VALUE:
                std::cerr << "Invalid value" << std::endl;
            case GL_INVALID_OPERATION:
                std::cerr << "Invalid operation" << std::endl;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                std::cerr << "Invalid framebuffer operation" << std::endl;
            case GL_OUT_OF_MEMORY:
                std::cerr << "Out of memory" << std::endl;
            default:
                std::cerr << "Unknown error" << std::endl;
            }
        }
    }

    void APIENTRY openglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
    {
        if (type == GL_DEBUG_TYPE_ERROR)
        {
            std::cerr << "OpenGL Error: " << message << std::endl;
        }
        else
        {
            std::cout << "OpenGL Message: " << message << std::endl;
        }
    }
}
