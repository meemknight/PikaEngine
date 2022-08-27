#include <iostream>
#include <glad/glad.h>
#include "dllMain.h"
#include <imgui.h>
#include "pikaImgui/pikaImgui.h"
#include <assert/assert.h>


PIKA_API void testStart(GLFWwindow *wind)
{
	//PIKA_PERMA_ASSERT(glfwInit(), "Problem initializing glfw from dll");
	//glfwMakeContextCurrent(wind);

	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad from dll");

	//pika::initImgui(wind);
}


PIKA_API void testUpdate(GLFWwindow *wind)
{
	pika::imguiStartFrame();



	ImGui::Begin("test");
	
	ImGui::End();


	pika::imguiEndFrame(wind);
}