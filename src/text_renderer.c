#include "text_renderer.h"

#include <string.h>

static void upload_projection(TextRenderer *r)
{
	mat4 projection;
	glm_ortho(0.0f, (float)r->width, 0.0f, (float)r->height, -1.0f, 1.0f, projection);
	shader_use(&r->shader);
	shader_set_mat4(&r->shader, "projection", (float *)projection);
}

bool text_renderer_init(TextRenderer *r, const Font *font, int w, int h)
{
	memset(r, 0, sizeof *r);
	r->font = font;
	r->width = w;
	r->height = h;

	r->shader = shader_create("../src/shaders/text.vs", "../src/shaders/text.fs", NULL);
	if (r->shader.id == 0) return false;

	upload_projection(r);


	glGenVertexArrays(1, &r->vao);
	glGenBuffers(1, &r->vbo);
	glBindVertexArray(r->vao);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	return true;	
}

void text_renderer_resize(TextRenderer *r, int w, int h)
{
	r->width = w;
	r->height = h;
	upload_projection(r);
}

void text_draw_cell(TextRenderer *r, char c, int col, int row, const vec3 color)
{
	const Glyph *g = &r->font->glyphs[(unsigned char)c];
	if (!g->tex) return;

	float cell_x = (float)(col * r->font->cell_w);
	float cell_top = (float)r->height - (float)(row * r->font->cell_h);
	float baseline = cell_top - (float)r->font->advance;


}

