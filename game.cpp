#include <cstdio>
#include "GLFW/glfw3.h"


// get error events before crating window
// events are reported through callbacks
GLFWerrorfun glfwSetErrorCallback(GLFWerrorfun cbfun);
// call back
// first arguments is the error code, second is description
typedef void(*GLFWerrorfun) (int, const char*);
