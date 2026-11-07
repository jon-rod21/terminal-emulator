#include <stdbool.h>

#define CHAR_LIMIT 128

typedef struct { unsigned int tex; int size[2], bearing[2]; unsigned int advance;} Glyph;
typedef struct { Glyph glyphs[CHAR_LIMIT]; int cell_w, cell_h; } Font;

bool font_load(Font *f, const char *path, int pixel_size);
