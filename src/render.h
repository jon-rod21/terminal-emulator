#ifndef RENDER_H
#define RENDER_H

#include <stdbool.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "shaders/shader.h"
#include "term.h"

#define CHAR_LIMIT 128

typedef struct {
	unsigned int TextureID;
	int Size[2];
	int Bearing[2];
	unsigned int Advance;
} Character;

typedef struct {
	GLFWwindow *window;
	Shader shader;
	Character characters[CHAR_LIMIT];
	unsigned int vao, vbo;
	int width, height;
} Renderer;

bool render_init(Renderer *r, int width, int height, const char *title);
void render_draw_screen(Renderer *r, const Screen *s);
void render_destroy(Renderer *r);

#endif
