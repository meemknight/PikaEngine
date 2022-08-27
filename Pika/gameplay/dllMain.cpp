#include <iostream>
#include <glad/glad.h>
#include "dllMain.h"
#include <imgui.h>
#include "pikaImgui/pikaImgui.h"
#include <assert/assert.h>

#include <gl2d/gl2d.h>
gl2d::Renderer2D renderer;


PIKA_API void testStart(pika::ImguiAndGlfwContext pikaContext)
{
	
	//PIKA_PERMA_ASSERT(glfwInit(), "Problem initializing glfw from dll");
	//glfwMakeContextCurrent(wind);

	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad from dll");
	//printf("%s\n", glGetString(GL_VERSION));
	//pika::initImgui(pikaContext);


	gl2d::init();
	renderer.create();

}

void *userMalloc(size_t sz, void *)
{
	return malloc(sz);
}

void userFree(void *ptr, void *)
{
	free(ptr);
}

PIKA_API void testUpdate(pika::ImguiAndGlfwContext pikaContext)
{
	gl2d::enableNecessaryGLFeatures();
	renderer.updateWindowMetrics(640, 480);
	renderer.renderRectangle({10,10, 100, 100}, Colors_Magenta);
	renderer.flush();

	//ImGui::SetCurrentContext(imguiContextGlobal);

//	ImGui::SetAllocatorFunctions(userMalloc, userFree);

	pika::setContext(pikaContext);

	ImGui::Begin("test");
	ImGui::End();

}