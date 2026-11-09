#include <unistd.h>
#include <stdbool.h>

typedef struct { int fd; pid_t pid; } Pty;

bool pty_spawn(Pty *p, const char *shell, int rows, int cols);
ssize_t pty_read(Pty *p, char *buf, size_t n);
bool pty_write(Pty *p, const char *buf, size_t n);
void pty_resize(Pty *p, int rows, int cols);
void pty_destroy(Pty *p);
