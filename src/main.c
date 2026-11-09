#include <stdio.h>
#include <stdlib.h>

#include "render.h"
#include "term.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BUF_SIZE 4096

int main(void)
{
	Renderer r;
	if (!render_init(&r, SCREEN_WIDTH, SCREEN_HEIGHT, "terminal-emulator"))
		return EXIT_FAILURE;

	Pty pty;
	if (!pty_spawn(&pty, "zsh", ROWS, COLS))
	{
		fprintf(stderr, "Failed to fork\n");
		render_destroy(&r);
		return EXIT_FAILURE;
	}
	pty_write(&pty, "ls -a\n", 6);

	Screen screen;
	screen_init(&screen);

	char buf[BUF_SIZE];

	while (!glfwWindowShouldClose(r.window))
	{
		ssize_t n = 0;
		size_t budget = 64 * 1024;
		while (budget > 0 && (n = pty_read(&pty, buf, sizeof(buf))) > 0)
		{
			for (ssize_t i = 0; i < n; i++)
				screen_put(&screen, (unsigned char)buf[i]);
			budget -= (size_t)n;
		}
		if (n < 0)
			glfwSetWindowShouldClose(r.window, GLFW_TRUE);   /* shell exited */

		if (glfwGetKey(r.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(r.window, GLFW_TRUE);

		glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		render_draw_screen(&r, &screen);

		glfwSwapBuffers(r.window);
		glfwPollEvents();
	}

	pty_destroy(&pty);
	render_destroy(&r);
	return EXIT_SUCCESS;
}
