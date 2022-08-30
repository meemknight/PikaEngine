#include "window.h"
#include <assert/assert.h>

void pika::PikaWindow::create()
{
	context.wind = glfwCreateWindow(640, 480, "Pika", NULL, NULL);
	
	PIKA_PERMA_ASSERT(context.wind, "problem initializing window");
	glfwMakeContextCurrent(context.wind);
}

bool pika::PikaWindow::shouldClose()
{
	return glfwWindowShouldClose(context.wind);
}

void pika::PikaWindow::update()
{
	glfwPollEvents();
	glfwSwapBuffers(context.wind);
}
