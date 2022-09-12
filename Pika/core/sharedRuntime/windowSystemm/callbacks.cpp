#include "callbacks.h"
#include "window.h"


void windowFocusCallback(GLFWwindow *window, int focused)
{
	auto ptr = glfwGetWindowUserPointer(window);
	pika::PikaWindow &pikaWindow = *(pika::PikaWindow *)ptr;

	if (focused)
	{
		pikaWindow.windowState.hasFocus = 1;
	}
	else
	{
		pikaWindow.windowState.hasFocus = 0;
	}
}

static void processAButton(pika::Button &b, int action)
{
	if (action == GLFW_PRESS)
	{
		b.setHeld(true);
	}
	else if (action == GLFW_RELEASE)
	{
		b.setHeld(false);
	}
	else if (action == GLFW_REPEAT)
	{
		b.setHeld(true);
		b.setTyped(true);
	}

};

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	auto ptr = glfwGetWindowUserPointer(window);
	pika::PikaWindow &pikaWindow = *(pika::PikaWindow *)ptr;

	if(key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
	{
		int index = key - GLFW_KEY_A;
		processAButton(pikaWindow.input.buttons[pika::Button::A + index], action);
	}else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
	{
		int index = key - GLFW_KEY_0;
		processAButton(pikaWindow.input.buttons[pika::Button::NR0 + index], action);
	}else
	{

		if (key == GLFW_KEY_SPACE)
		{
			processAButton(pikaWindow.input.buttons[pika::Button::Space], action);
		}
		else
		if (key == GLFW_KEY_ENTER)
		{
			processAButton(pikaWindow.input.buttons[pika::Button::Enter], action);
		}
		else
		if (key == GLFW_KEY_ESCAPE)
		{
			processAButton(pikaWindow.input.buttons[pika::Button::Escape], action);
		}
		else
		if (key == GLFW_KEY_UP)
		{
			processAButton(pikaWindow.input.buttons[pika::Button::Up], action);
		}
		else
		if (key == GLFW_KEY_DOWN)
		{
			processAButton(pikaWindow.input.buttons[pika::Button::Down], action);
		}
		else
		if (key == GLFW_KEY_LEFT)
		{
			processAButton(pikaWindow.input.buttons[pika::Button::Left], action);
		}
		else
		if (key == GLFW_KEY_RIGHT)
		{
			processAButton(pikaWindow.input.buttons[pika::Button::Right], action);
		}
		else
		if (key == GLFW_KEY_LEFT_CONTROL)
		{
			processAButton(pikaWindow.input.buttons[pika::Button::LeftCtrl], action);
		}else
		if (key == GLFW_KEY_TAB)
		{
			processAButton(pikaWindow.input.buttons[pika::Button::Tab], action);
		}else
		if (key == GLFW_KEY_LEFT_ALT)
		{
			processAButton(pikaWindow.input.buttons[pika::Button::LeftAlt], action);
		}
	}

}

void mouseCallback(GLFWwindow *window, int key, int action, int mods)
{

	auto ptr = glfwGetWindowUserPointer(window);
	pika::PikaWindow &pikaWindow = *(pika::PikaWindow *)ptr;

	if (key == GLFW_MOUSE_BUTTON_LEFT)
	{
		processAButton(pikaWindow.input.lMouse, action);
	}
	else if(key == GLFW_MOUSE_BUTTON_RIGHT)
	{
		processAButton(pikaWindow.input.rMouse, action);
	}


}
