#define ROWS 24
#define COLS 80

typedef struct {
	char cells[ROWS][COLS + 1];
	int row, col;
} Screen;

void screen_init(Screen *s);
void screen_scroll(Screen *s);
void screen_put(Screen *s, unsigned char c);

