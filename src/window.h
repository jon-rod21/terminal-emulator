#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>


typedef struct { GLFWwindow *handle; int width, height; } Window;

bool window_init(Window *w, int width, int height, const char *title);
bool window_destroy(Window *w);
