#include <stdbool.h>
#include <stdio.h>
#include "window.h"

#define MAJOR 4
#define MINOR 6

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

static void error_callback(int code, const char *description)
{
	fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

bool window_init(Window *w, int width, int height, const char *title)
{
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to init GLFW\n");
		return false;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	w->handle = glfwCreateWindow(width, height, title, NULL, NULL);

	if (!w->handle)
	{
		fprintf(stderr, "Failed to create window\n");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(w->handle);
	glfwSetFramebufferSizeCallback(w->handle, framebuffer_size_callback);
	
	int version = gladLoadGL(glfwGetProcAddress);
	if (!version)
	{
		fprintf(stderr, "Failed to load OpenGL\n");
		glfwTerminate();
		return false;
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;

}
