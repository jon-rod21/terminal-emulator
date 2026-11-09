#include "render.h"

#include <stdio.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cglm/cglm.h>

#define MAJOR 4
#define MINOR 6


// window
static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

static void error_callback(int code, const char *description)
{
	fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

static bool window_init(Renderer *r, const char *title)
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

	r->window = glfwCreateWindow(r->width, r->height, title, NULL, NULL);
	if (!r->window)
	{
		fprintf(stderr, "Failed to create window\n");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(r->window);
	glfwSetFramebufferSizeCallback(r->window, framebuffer_size_callback);

	int version = gladLoadGL(glfwGetProcAddress);
	if (!version)
	{
		fprintf(stderr, "Failed to load OpenGL\n");
		glfwDestroyWindow(r->window);
		glfwTerminate();
		return false;
	}
	printf("OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
	return true;
}


// font
static bool font_load(Renderer *r, const char *path, int pixel_size)
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		fprintf(stderr, "Failed to init Freetype library\n");
		return false;
	}

	FT_Face face;
	if (FT_New_Face(ft, path, 0, &face))
	{
		fprintf(stderr, "Failed to load font\n");
		FT_Done_FreeType(ft);
		return false;
	}

	FT_Set_Pixel_Sizes(face, 0, pixel_size);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (unsigned char c = 0; c < CHAR_LIMIT; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			fprintf(stderr, "Failed to load glyph\n");
			continue;
		}

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0, GL_RED, GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		r->characters[c].TextureID  = texture;
		r->characters[c].Size[0]    = face->glyph->bitmap.width;
		r->characters[c].Size[1]    = face->glyph->bitmap.rows;
		r->characters[c].Bearing[0] = face->glyph->bitmap_left;
		r->characters[c].Bearing[1] = face->glyph->bitmap_top;
		r->characters[c].Advance    = face->glyph->advance.x;
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	return true;
}

// draw
static void render_text(const Renderer *r, const char *text,
						float x, float y, float scale,
						float cr, float cg, float cb)
{
	shader_use(&r->shader);
	shader_set_vec3(&r->shader, "textColor", cr, cg, cb);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(r->vao);

	for (const char *c = text; *c != '\0'; c++)
	{
		Character ch = r->characters[(unsigned char)*c];
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
		glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.Advance >> 6) * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void render_draw_screen(Renderer *r, const Screen *s)
{
	float y = (float)r->height;
	for (int row = 0; row < ROWS; row++)
	{
		render_text(r, s->cells[row], 0.0f, y, 0.3f, 1.0f, 1.0f, 1.0f);
		y -= 25.0f;
	}
}

// inits/destroys
bool render_init(Renderer *r, int width, int height, const char *title)
{
	memset(r, 0, sizeof(*r));
	r->width = width;
	r->height = height;

	if (!window_init(r, title)) return false;

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	r->shader = shader_create("../src/shaders/text.vs", "../src/shaders/text.fs", NULL);
	if (r->shader.id == 0) goto fail_window;

	mat4 projection;
	glm_ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f, projection);
	shader_use(&r->shader);
	shader_set_mat4(&r->shader, "projection", (float *)projection);

	if (!font_load(r, "../src/fonts/Monocraft.ttf", 48)) goto fail_window;

	glGenVertexArrays(1, &r->vao);
	glGenBuffers(1, &r->vbo);
	glBindVertexArray(r->vao);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return true;

fail_window:
	glfwDestroyWindow(r->window);
	glfwTerminate();
	return false;
}

void render_destroy(Renderer *r)
{
	for (int c = 0; c < CHAR_LIMIT; c++)
		if (r->characters[c].TextureID)
			glDeleteTextures(1, &r->characters[c].TextureID);
	glDeleteBuffers(1, &r->vbo);
	glDeleteVertexArrays(1, &r->vao);
	glDeleteProgram(r->shader.id);

	glfwDestroyWindow(r->window);
	glfwTerminate();
	memset(r, 0, sizeof(*r));
}
