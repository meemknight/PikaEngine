#include <iostream>
#include <glad/glad.h>
#include "dllMain.h"
#include <imgui.h>
#include "pikaImgui/pikaImgui.h"
#include <assert/assert.h>

#include <gl2d/gl2d.h>
gl2d::Renderer2D renderer;

ImGuiContext *imguiContextGlobal = 0;

PIKA_API void testStart(GLFWwindow *wind, ImGuiContext *imguiContext)
{
	
	//PIKA_PERMA_ASSERT(glfwInit(), "Problem initializing glfw from dll");
	//glfwMakeContextCurrent(wind);

	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad from dll");
	//printf("%s\n", glGetString(GL_VERSION));
	//pika::initImgui(wind);

	imguiContextGlobal = imguiContext;
	ImGui::SetCurrentContext(imguiContextGlobal);
	//ImGui::SetAllocatorFunctions()

	gl2d::init();
	renderer.create();

}


PIKA_API void testUpdate(GLFWwindow *wind)
{
	gl2d::enableNecessaryGLFeatures();
	renderer.updateWindowMetrics(640, 480);
	renderer.renderRectangle({10,10, 100, 100}, Colors_Magenta);
	renderer.flush();

	ImGui::SetCurrentContext(imguiContextGlobal);

	pika::imguiStartFrame();
	ImGui::Begin("test");
	ImGui::End();
	pika::imguiEndFrame(wind);
}