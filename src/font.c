#include "font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

bool font_load(Font *f, const char *path, int pixel_size)
{
	memset(f, 0, sizeof(*f));

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		fprintf(stderr, "Failed to init FreeType library");
		return false;
	}

	FT_Face face;
	if (FT_New_Face(ft, path, 0, &face))
	{
		FT_Done_FreeType(ft);
		fprintf(stderr, "Failed to load font");
		return EXIT_FAILURE;
	}
	else
	{
		FT_Set_Pixel_Sizes(face, 0, pixel_size);
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

			f->glyphs[c].tex = texture;
			f->glyphs[c].size[0] = face->glyph->bitmap.width;
			f->glyphs[c].size[1] = face->glyph->bitmap.rows;
			f->glyphs[c].bearing[0] = face->glyph->bitmap_left;
			f->glyphs[c].bearing[1] = face->glyph->bitmap_top;
			f->glyphs[c].advance = face->glyph->advance.x;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	f->cell_w = f->glyphs['M'].advance >> 6;
	f->cell_h = face->size->metrics.height >> 6;

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	return true;
	
}

void font_destroy(Font *f)
{
	for (int c = 0; c < CHAR_LIMIT; c++)
		if (f->glyphs[c].tex)
			glDeleteTextures(1, &f->glyphs[c].tex);
	memset(f, 0, sizeof *f);
}
