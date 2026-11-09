#include "term.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pty.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// screen
void screen_init(Screen *s)
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

void screen_put(Screen *s, unsigned char c)
{
	if (c == '\n')      s->row++;
	else if (c == '\r') s->col = 0;
	else if (c == '\b') { if (s->col > 0) s->col--; }
	else if (c >= 32 && c < 127)
	{
		if (s->col >= COLS) { s->col = 0; s->row++; }
		if (s->row >= ROWS) screen_scroll(s);
		s->cells[s->row][s->col++] = c;
	}
	if (s->row >= ROWS) screen_scroll(s);
}

// pty
bool pty_spawn(Pty *p, const char *shell, int rows, int cols)
{
	memset(p, 0, sizeof(*p));
	p->fd = -1;

	struct winsize ws = { .ws_row = rows, .ws_col = cols };
	pid_t pid = forkpty(&p->fd, NULL, NULL, &ws);
	if (pid < 0) return false;

	if (pid == 0)
	{
		execlp(shell, shell, "-f", (char *)NULL);
		_exit(127);
	}
	p->pid = pid;

	int flags = fcntl(p->fd, F_GETFL, 0);
	if (flags < 0 || fcntl(p->fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		pty_destroy(p);
		return false;
	}
	return true;
}

ssize_t pty_read(Pty *p, char *buf, size_t n)
{
	ssize_t r = read(p->fd, buf, n);
	if (r > 0)  return r;
	if (r == 0) return -1;
	if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) return 0;
	return -1;   /* EIO: child exited */
}

bool pty_write(Pty *p, const char *buf, size_t n)
{
	size_t off = 0;
	while (off < n)
	{
		ssize_t w = write(p->fd, buf + off, n - off);
		if (w > 0) { off += (size_t)w; continue; }
		if (w < 0 && errno == EINTR) continue;
		if (w < 0 && errno == EAGAIN)
		{
			struct pollfd pfd = { .fd = p->fd, .events = POLLOUT };
			poll(&pfd, 1, 100);
			continue;
		}
		return false;
	}
	return true;
}

void pty_destroy(Pty *p)
{
	if (p->fd >= 0) close(p->fd);
	if (p->pid > 0)
	{
		kill(p->pid, SIGHUP);
		waitpid(p->pid, NULL, 0);
	}
	p->fd = -1;
	p->pid = 0;
}
