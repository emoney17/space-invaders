#include <cstdio>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

// get error events before crating window
// events are reported through callbacks
GLFWerrorfun glfwSetErrorCallback(GLFWerrorfun cbfun);
// call back
// first arguments is the error code, second is description
typedef void(*GLFWerrorfun) (int, const char*);

// print the error
void errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main()
{
    // hand over error callback to GLFW
    glfwSetErrorCallback(errorCallback);

    // initialize glfw
    if (!glfwInit())
    {
        return -1;
    }

    // create a window
    GLFWwindow* window = glfwCreateWindow(640, 480, "Space Invaders", NULL, NULL);
    if (!window)
    {
        glfwTerminate(); // destroy resources used
        return -1;
    }
    // make call future calls to the context: window
    glfwMakeContextCurrent(window);

    // set glfw hints for version
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // initialize glew
    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        printf("Error initializing GLFW\n");
        glfwTerminate();
        return -1;
    }

    // query opengl version
    int glVersion[2] = {-1, 1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);

    // game loop, keep the window open
    glClearColor(1.0, 0.0, 0.0, 1.0); // set to red
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window); //swap front and back buffers (display and drawing)
        glfwPollEvents(); // process any pending events
    }

    // exit cleanly
    glfwDestroyWindow(window);
    glfwTerminate();
}
