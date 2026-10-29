#include <stdio.h>
#include <stdlib.h>
#include <pty.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include "shaders/shader.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAJOR 4
#define MINOR 6
#define CHAR_LIMIT 128

typedef struct  {
	unsigned int TextureID;
	int Size;
	int Bearing;
	unsigned int Advance;
}Character;

Character characters[CHAR_LIMIT];
unsigned int VAO, VBO;

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

static void error_callback(int code, const char *description)
{
	fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

int main(void)
{

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to init GLFW\n");
		return EXIT_FAILURE;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "terminal-emulator", NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "Failed to create window\n");
		glfwTerminate();
		return EXIT_FAILURE;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	int version = gladLoadGL(glfwGetProcAddress);
	if (!version)
	{
		fprintf(stderr, "Failed to load OpenGL\n");
		glfwTerminate();
		return EXIT_FAILURE;
	}
	printf("OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	Shader text_shader = shader_create("../src/shaders/text.vs", "../src/shaders/text.fs", NULL);
	if (text_shader.id == 0) return EXIT_FAILURE;

	mat4 projection;
	glm_ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT, -1.0f, 1.0f, projection);
	shader_use(&text_shader);
	shader_set_mat4(&text_shader, "projection", (float *)projection);

//	int primary_fd;
//	struct termios termp;
//	struct winsize winp;
//
//
//	pid_t pid = forkpty(&primary_fd, NULL, &termp, &winp);
//	if (pid == -1)
//	{
//		fprintf(stderr, "Error creating pty");
//		return EXIT_FAILURE;
//	}

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		fprintf(stderr, "Failed to init Freetype library");
		return EXIT_FAILURE;
	}

	FT_Face face;
	if (FT_New_Face(ft, "../src/arial-regular-49_ufonts.com.ttf", 0, &face))
	{
		fprintf(stderr, "Failed to load font");
		return EXIT_FAILURE;
	}
	else
	{
		FT_Set_Pixel_Sizes(face, 0, 48);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		for (unsigned char c = 0; c < CHAR_LIMIT; c++)
		{
			// load char glyph

			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				fprintf(stderr, "Failed to load glyph");
				continue;
			}

			// generate texture;
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
					GL_TEXTURE_2D,
					0,
					GL_RED,
					face->glyph->bitmap.width,
					face->glyph->bitmap.rows,
					0,
					GL_RED,
					GL_UNSIGNED_BYTE,
					face->glyph->bitmap.buffer
			);
		

			// set tex options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			characters[c].TextureID = texture;
			characters[c].Size = face->glyph->bitmap.width, face->glyph->bitmap.rows;
			characters[c].Bearing = face->glyph->bitmap.width, face->glyph->bitmap.rows;
			characters[c].Advance = face->glyph->advance.x;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	// destroy freetype once done
	FT_Done_Face(face);
	FT_Done_FreeType(ft);


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);



	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// draw lol
		
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;

}

