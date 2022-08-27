#include <iostream>
#include <cstdio>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include "assert/assert.h"
#include "dllLoader/dllLoader.h"
#include "pikaImgui/pikaImgui.h"

int main()
{
	std::filesystem::path currentPath = std::filesystem::current_path();
	
	
	testStart_t *testStart = {};
	testUpdate_t *testUpdate = {};

	PIKA_PERMA_ASSERT(pika::loadDll(currentPath, &testStart, &testUpdate), "Couldn't load dll");
	
	PIKA_PERMA_ASSERT(glfwInit(), "Problem initializing glfw");

	//glfwSetErrorCallback(error_callback); todo

	pika::PikaContext context = {};

	context.wind = glfwCreateWindow(640, 480, "Pika", NULL, NULL);
	if (!context.wind)
	{
		std::cout << "problem initializing window";
	}

	glfwMakeContextCurrent(context.wind);

	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad");

	context.ImGuiContext = pika::initImgui(context);

	context.glfwMakeContextCurrentPtr = glfwMakeContextCurrent;

	testStart(context);

	while (!glfwWindowShouldClose(context.wind))
	{

		glClear(GL_COLOR_BUFFER_BIT);

		pika::imguiStartFrame(context);

		testUpdate(context);

		pika::imguiEndFrame(context);



		glfwPollEvents();
		glfwSwapBuffers(context.wind);
	}



	return 0;
}