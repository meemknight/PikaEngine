#include "window.h"
#include <logs/assert.h>
#include "callbacks.h"

#ifdef PIKA_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <Windows.h>
#include <GLFW/glfw3native.h>
#endif

#include <pikaSizes.h>

#include <safeSave.h>

struct WindowRect
{
	int x = 100, y = 100, z = 640, w = 480;
};

void pika::PikaWindow::create()
{

	WindowRect wr = {};

#if PIKA_DEVELOPMENT
	if (sfs::safeLoad(&wr, sizeof(wr), PIKA_ENGINE_SAVES_PATH "windowPos", false) != sfs::noError)
	{
		wr = {};
	}
#endif
	
	if (wr.x < 0 || wr.y < 0 || wr.z <= 0 || wr.w <= 0)
	{
		wr = {};
	}
	
	//todo debug from engine
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	context.wind = glfwCreateWindow(wr.z, wr.w, "Pika", NULL, NULL);
	glfwSetWindowPos(context.wind, wr.x, wr.y);

	input.hasFocus = true;

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

void pika::PikaWindow::saveWindowPositions()
{
#if PIKA_DEVELOPMENT

	WindowRect wr = {};

	glfwGetWindowPos(context.wind, &wr.x, &wr.y);

	wr.z = windowState.w;
	wr.w = windowState.h;

	sfs::safeSave(&wr, sizeof(wr), PIKA_ENGINE_SAVES_PATH "windowPos", false);

#endif

}

bool pika::PikaWindow::shouldClose()
{
	return glfwWindowShouldClose(context.wind);
}

void pika::PikaWindow::update()
{
#pragma region deltaTime
	auto end = std::chrono::high_resolution_clock::now();
	input.deltaTime = (std::chrono::duration_cast<std::chrono::microseconds>(end - timer)).count() / 1000000.0f;
	timer = end;

	if (input.deltaTime > 1.f / 10) { input.deltaTime = 1.f / 10; }
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
			b.setTyped(true);
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
