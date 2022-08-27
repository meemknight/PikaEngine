#include <iostream>
#include <cstdio>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl2d/gl2d.h>
gl2d::Renderer2D renderer;

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

	GLFWwindow *window = glfwCreateWindow(640, 480, "Pika", NULL, NULL);
	if (!window)
	{
		std::cout << "problem initializing window";
	}

	glfwMakeContextCurrent(window);

	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad");

	gl2d::init();
	renderer.create();

	pika::initImgui(window);

	testStart(window);

	while (!glfwWindowShouldClose(window))
	{

		glClear(GL_COLOR_BUFFER_BIT);
		pika::imguiStartFrame();

		gl2d::enableNecessaryGLFeatures();
		renderer.updateWindowMetrics(640, 480);
		renderer.renderRectangle({10,10, 100, 100}, Colors_Magenta);
		renderer.flush();

		testUpdate(window);


		pika::imguiEndFrame(window);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}



	return 0;
}