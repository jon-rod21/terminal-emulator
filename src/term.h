#ifndef TERM_H
#define TERM_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define ROWS 24
#define COLS 80

typedef struct {
	char cells[ROWS][COLS + 1];   
	int row, col;                 // cursor 
} Screen;

void screen_init(Screen *s);
void screen_put(Screen *s, unsigned char c);

typedef struct {
	int fd;
	pid_t pid;
} Pty;

bool    pty_spawn(Pty *p, const char *shell, int rows, int cols);
ssize_t pty_read(Pty *p, char *buf, size_t n);   // >0 bytes, 0 nothing yet, -1 closed 
bool    pty_write(Pty *p, const char *buf, size_t n);
void    pty_destroy(Pty *p);

#endif
