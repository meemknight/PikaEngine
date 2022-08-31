#include "callbacks.h"
#include "window.h"

void mouseCallback(GLFWwindow *window, int key, int action, int mods)
{

	auto ptr = glfwGetWindowUserPointer(window);
	pika::PikaWindow &pikaWindow = *(pika::PikaWindow *)ptr;

	bool state = 0;
	if (action == GLFW_PRESS)
	{
		state = 1;
	}
	else if (action == GLFW_RELEASE)
	{
		state = 0;
	}
	else
	{
		return; //we don't care about any other actions
	}

	if (key == GLFW_MOUSE_BUTTON_LEFT)
	{
		pikaWindow.input.lMouse.setHeld(state);
	}
	else if(key == GLFW_MOUSE_BUTTON_RIGHT)
	{
		pikaWindow.input.rMouse.setHeld(state);
	}


}