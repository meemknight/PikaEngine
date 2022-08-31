#include "window.h"
#include <assert/assert.h>
#include "callbacks.h"

void pika::PikaWindow::create()
{
	context.wind = glfwCreateWindow(640, 480, "Pika", NULL, NULL);
	
	PIKA_PERMA_ASSERT(context.wind, "problem initializing window");
	glfwMakeContextCurrent(context.wind);

	glfwSetWindowUserPointer(context.wind, this);

	glfwSetMouseButtonCallback(context.wind, mouseCallback);

}

bool pika::PikaWindow::shouldClose()
{
	return glfwWindowShouldClose(context.wind);
}

void pika::PikaWindow::update()
{


	glfwPollEvents();
	glfwSwapBuffers(context.wind);


#pragma region input

	auto processInput = [](pika::Button &b)
	{

		if (!b.lastState() && b.held())
		{
			b.setPressed(true);
		}
		else
		{
			b.setPressed(false);
		}

		if (b.lastState() && !b.held())
		{
			b.setReleased(true);
		}
		else
		{
			b.setReleased(false);
		}

		b.setLastState(b.held());

	};

	processInput(input.lMouse);
	processInput(input.rMouse);


#pragma endregion
}
