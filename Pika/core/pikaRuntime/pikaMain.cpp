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

	GLFWwindow *window = glfwCreateWindow(640, 480, "Pika", NULL, NULL);
	if (!window)
	{
		std::cout << "problem initializing window";
	}

	glfwMakeContextCurrent(window);

	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad");

	auto imguiContext = pika::initImgui(window);

	testStart(window, imguiContext);

	while (!glfwWindowShouldClose(window))
	{

		glClear(GL_COLOR_BUFFER_BIT);

		testUpdate(window);

		//pika::imguiStartFrame();
		//ImGui::Begin("test");
		//ImGui::End();
		//pika::imguiEndFrame(window);


		glfwPollEvents();
		glfwSwapBuffers(window);
	}



	return 0;
}