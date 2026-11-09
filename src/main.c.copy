#include <stdio.h>
#include <stdlib.h>
#include <pty.h>
#include <unistd.h>
#include <poll.h>
#include <sys/wait.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include "shaders/shader.h"
#include "screen.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAJOR 4
#define MINOR 6
#define CHAR_LIMIT 128
#define BUF_SIZE 4096

typedef struct  {
	unsigned int TextureID;
	int Size[2];
	int Bearing[2];
	unsigned int Advance;
}Character;

Character characters[CHAR_LIMIT];
unsigned int VAO, VBO;

void render_text(const Shader *shader, const char *text,
				 float x, float y, float scale,
				 float r, float g, float b);

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

	// --- window init ---
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
	// --- window init ---

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);






	Shader text_shader = shader_create("../src/shaders/text.vs", "../src/shaders/text.fs", NULL);
	if (text_shader.id == 0) return EXIT_FAILURE;

	mat4 projection;
	glm_ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT, -1.0f, 1.0f, projection);
	shader_use(&text_shader);
	shader_set_mat4(&text_shader, "projection", (float *)projection);


	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		fprintf(stderr, "Failed to init Freetype library");
		return EXIT_FAILURE;
	}

	FT_Face face;
	if (FT_New_Face(ft, "../src/Monocraft.ttf", 0, &face))
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
			characters[c].Size[0] = face->glyph->bitmap.width;
			characters[c].Size[1] = face->glyph->bitmap.rows;
			characters[c].Bearing[0] = face->glyph->bitmap_left;
			characters[c].Bearing[1] = face->glyph->bitmap_top;
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


	
	// start the pty
	int amaster;
	struct winsize winp = { .ws_row = 24, .ws_col = 80 };

	pid_t pid = forkpty(&amaster, NULL, NULL, &winp);
	if (pid < 0)
	{
		fprintf(stderr, "Failed to fork");
		return -1;
	}
	if (pid == 0)
	{
		execlp("zsh", "zsh", "-f", (char *)NULL);
		_exit(127);
	}

	write(amaster, "ls -a\n", 6);

	char buf[BUF_SIZE] = {0};
	struct pollfd pfd = { .fd = amaster, .events = POLLIN };

	Screen screen;
	screen_init(&screen);

	while (!glfwWindowShouldClose(window))
	{
		while (poll(&pfd, 1, 0) > 0)
		{
			ssize_t n = read(amaster, buf, sizeof(buf));
			if (n <= 0) break;
			//fwrite(buf, 1, n, stdout);
			for (ssize_t i = 0; i < n; i++)
				screen_put(&screen, (unsigned char)buf[i]);

		}
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		float y = SCREEN_HEIGHT;
		for (int r = 0; r < ROWS; r++)
		{
			render_text(&text_shader, screen.cells[r], 0.0f, y, 0.3f, 1.0f, 1.0f, 1.0f);
			y -= 25.0f;
		}

		
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	close(amaster);
	waitpid(pid, NULL, 0);

	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;

}


void render_text(const Shader *shader, const char *text,
				 float x, float y, float scale,
				 float r, float g, float b)
{
	shader_use(shader);
	shader_set_vec3(shader, "textColor", r, g, b);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	for (const char *c = text; *c != '\0'; c++)
	{
		Character ch = characters[(unsigned char)*c];
		float xpos = x + ch.Bearing[0] * scale;
		float ypos = y - (ch.Size[1] - ch.Bearing[1]) * scale;
		float w = ch.Size[0] * scale;
		float h = ch.Size[1] * scale;

		float vertices[6][4] = {
			{ xpos,     ypos + h, 0.0f, 0.0f },
			{ xpos,     ypos,     0.0f, 1.0f },
			{ xpos + w, ypos,     1.0f, 1.0f },

			{ xpos,     ypos + h, 0.0f, 0.0f },
			{ xpos + w, ypos,     1.0f, 1.0f },
			{ xpos + w, ypos + h, 1.0f, 0.0f },
		};

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.Advance >> 6) * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

