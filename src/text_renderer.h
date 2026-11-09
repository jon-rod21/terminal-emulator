#include "shaders/shader.h"
#include "font.h"
#include "screen.h"
#include <cglm/cglm.h>

typedef struct {
	Shader shader;
	unsigned int vao, vbo;
	const Font *font;
	int width, height;
} TextRenderer;

bool text_renderer_init(TextRenderer *r, const Font *font, int w, int h);
void text_renderer_resize(TextRenderer *r, int w, int h);
void text_draw(TextRenderer *r, const char *s, float x, float y, vec3 color);
void text_draw_screen(TextRenderer *r, const Screen *s);
void text_renderer_destroy(TextRenderer *r);

