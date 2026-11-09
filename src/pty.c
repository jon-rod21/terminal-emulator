#include "pty.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <pty.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
bool pty_spawn(Pty *p, const char *shell, int rows, int cols)
{
	memset(p, 0, sizeof(*p));
	struct winsize winp = { .ws_row = rows, .ws_col = cols };
	p->pid = forkpty(&p->fd, NULL, NULL, &winp);

	if (p->pid < 0)
	{
		fprintf(stderr, "Failed to fork");
		return false;
	}

	if (p->pid == 0)
	{
		execlp(shell, shell, "-f", (char *)NULL);
		_exit(127);
	}
	return true;
}

ssize_t pty_read(Pty *p, char *buf, size_t n)
{
	ssize_t r = read(p->fd, buf, n);
	if (r > 0) return r;
	if (r == 0) return -1;
	if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		return 0;
	return -1;
}

bool pty_write(Pty *p, const char *buf, size_t n)
{
	size_t off = 0;
	while (off < n) 
	{
		ssize_t w = write(p->fd, buf + off, n - off);
		if (w > 0) { off += w; continue; }
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

void pty_resize(Pty *p, int rows, int cols)
{
	struct winsize ws = { .ws_row = rows, .ws_col = cols };
	ioctl(p->fd, TIOCSWINSZ, &ws);
}

void pty_destroy(Pty *p)
{
	if (p->fd >= 0) close (p->fd);
	if (p->pid > 0)
	{
		kill(p->pid, SIGHUP);
		waitpid(p->pid, NULL, 0);
	}
	p->fd = -1;
	p->pid = 0;
}




