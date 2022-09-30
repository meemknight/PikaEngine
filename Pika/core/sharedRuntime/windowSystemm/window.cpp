#include "window.h"
#include <logs/assert.h>
#include "callbacks.h"

#ifdef PIKA_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <Windows.h>
#include <GLFW/glfw3native.h>
#endif

#include <pikaSizes.h>

void pika::PikaWindow::create()
{
	context.wind = glfwCreateWindow(640, 480, "Pika", NULL, NULL);
	
	windowState.hasFocus = true;

	PIKA_PERMA_ASSERT(context.wind, "problem initializing window");
	glfwMakeContextCurrent(context.wind);

	glfwSetWindowUserPointer(context.wind, this);

	glfwSetMouseButtonCallback(context.wind, mouseCallback);
	glfwSetWindowFocusCallback(context.wind, windowFocusCallback);
	glfwSetKeyCallback(context.wind, keyCallback);

	//todo macro
	context.imguiAllocator.init(malloc(pika::MB(20)), pika::MB(20));

	//HWND hwnd = glfwGetWin32Window(context.wind);

	//LONG exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	//exStyle &= ~WS_EX_APPWINDOW;
	//exStyle |= WS_EX_TOOLWINDOW;
	//exStyle |= WS_EX_CONTEXTHELP;
	//exStyle &= ~WS_MAXIMIZEBOX;
	//exStyle &= ~WS_MINIMIZEBOX;
	//SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

	//LONG style = GetWindowLongPtr(hwnd, GWL_STYLE);
	//style &= ~WS_MAXIMIZEBOX;
	//style &= ~WS_MINIMIZEBOX;
	//style &= ~WS_CAPTION;
	//style |= WS_DLGFRAME;
	//SetWindowLongPtr(hwnd, GWL_STYLE, style);
	


	timer = std::chrono::high_resolution_clock::now();
}

bool pika::PikaWindow::shouldClose()
{
	return glfwWindowShouldClose(context.wind);
}

void pika::PikaWindow::update()
{
#pragma region deltaTime
	auto end = std::chrono::high_resolution_clock::now();
	deltaTime = (std::chrono::duration_cast<std::chrono::microseconds>(end - timer)).count() / 1000000.0f;
	timer = end;

	if (deltaTime > 1.f / 10) { deltaTime = 1.f / 10; }
#pragma endregion

#pragma region input

	auto processInputBefore = [](pika::Button &b)
	{
		b.setTyped(false);
	};

	processInputBefore(input.lMouse);
	processInputBefore(input.rMouse);

	for (int i = 0; i < Button::BUTTONS_COUNT; i++)
	{
		processInputBefore(input.buttons[i]);
	}

#pragma endregion

	glfwPollEvents();
	glfwSwapBuffers(context.wind);

#pragma region window state

	{
		int w = 0;
		int h = 0;
		glfwGetWindowSize(context.wind, &w, &h);

		windowState.w = w;
		windowState.h = h;

	}

#pragma endregion


#pragma region input

	double mouseX = 0;
	double mouseY = 0;
	glfwGetCursorPos(context.wind, &mouseX, &mouseY);

	input.mouseX = (int)mouseX;
	input.mouseY = (int)mouseY;

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

	for (int i = 0; i < Button::BUTTONS_COUNT; i++)
	{
		processInput(input.buttons[i]);
	}


#pragma endregion

}
