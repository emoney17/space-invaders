#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

// get error events before crating window
// events are reported through callbacks
GLFWerrorfun glfwSetErrorCallback(GLFWerrorfun cbfun);
// call back
// first arguments is the error code, second is description
typedef void(*GLFWerrorfun) (int, const char*);

// print the error
void errorCallback(int error, std::string description)
{
    std::cout << "Error: " << description << std::endl;
}

glfwSetErrorCallback(errorCallback);

int main()
{
    // initialize glfw
    if (!glfwInit())
    {
        return -1;
    }

    // create a window
    window = glfwCreateWindow(640, 480, "Space Invaders", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    // make call future calls to the context: window
    glfwMakeContextCurrent(window);

    // set glfw hints for version
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}
