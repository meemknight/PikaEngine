#include <iostream>
//#include <glad/glad.h>
#include <glew/glew.h>
#include <gl2d/gl2d.h>
#include "dllMain.h"
//#include <imgui.h>
#include "pikaImgui/pikaImgui.h"
#include <assert/assert.h>

gl2d::Renderer2D renderer;


PIKA_API void testStart(GLFWwindow *wind)
{
	//PIKA_PERMA_ASSERT(glfwInit(), "Problem initializing glfw from dll");
	//glfwMakeContextCurrent(wind);

	//PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad from dll");
	PIKA_PERMA_ASSERT(glewInit() == GLEW_OK, "Problem initializing glew");

	gl2d::init();
	renderer.create();

	//pika::initImgui(wind);
}


PIKA_API void testUpdate(GLFWwindow *wind)
{

	gl2d::enableNecessaryGLFeatures();
	renderer.updateWindowMetrics(640, 480);
	renderer.renderRectangle({10,10, 100, 100}, Colors_Magenta);
	renderer.flush();

	//pika::imguiStartFrame();
	//ImGui::Begin("test");
	//ImGui::End();
	//pika::imguiEndFrame(wind);
}