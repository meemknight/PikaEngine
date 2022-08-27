#include <iostream>
#include <cstdio>
#include <filesystem>

//#include <glad/glad.h>
#include <glew/glew.h>
#include <gl2d/gl2d.h>

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
	
#pragma region window loader
	PIKA_PERMA_ASSERT(glfwInit(), "Problem initializing glfw");

	//glfwSetErrorCallback(error_callback); todo

	GLFWwindow *window = glfwCreateWindow(640, 480, "Pika", NULL, NULL);
	if (!window)
	{
		std::cout << "problem initializing window";
	}
	glfwMakeContextCurrent(window);
#pragma endregion


#pragma region opengl loader

	//PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad");
	PIKA_PERMA_ASSERT(glewInit() == GLEW_OK, "Problem initializing glew");

#pragma endregion



	pika::initImgui(window);

	testStart(window);

	while (!glfwWindowShouldClose(window))
	{

		glClear(GL_COLOR_BUFFER_BIT);
		pika::imguiStartFrame();

		testUpdate(window);


		pika::imguiEndFrame(window);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}



	return 0;
}