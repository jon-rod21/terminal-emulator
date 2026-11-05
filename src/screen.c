#include "screen.h"
#include <string.h>

static void screen_init(Screen *s)
{
	memset(s, 0, sizeof(*s));
	for (int r = 0; r < ROWS; r++)
		memset(s->cells[r], ' ', COLS);
}

static void screen_scroll(Screen *s)
{
	memmove(s->cells[0], s->cells[1], (ROWS - 1) * (COLS + 1));
	memset(s->cells[ROWS - 1], ' ', COLS);
	s->cells[ROWS - 1][COLS] = '\0';
	s->row = ROWS - 1;
}

static void screen_put(Screen *s, unsigned char c)
{
	if (c == '\n') { s->row++; }
	else if (c == '\r') { s->col = 0; }
	else if (c == '\b') { if (s->col > 0) s->col--; }
	else if (c >= 32 && c < 127) {
		if (s->col >= COLS) { s->col = 0; s->row++; }
		if (s->row >= ROWS) screen_scroll(s);
		s->cells[s->row][s->col++] = c;
	}
	if (s->row >= ROWS) screen_scroll(s);
}
