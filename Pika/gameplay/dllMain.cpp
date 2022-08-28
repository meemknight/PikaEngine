#include <iostream>
#include <glad/glad.h>
#include "dllMain.h"
#include <imgui.h>
#include "pikaImgui/pikaImgui.h"
#include <assert/assert.h>

#include <gl2d/gl2d.h>
gl2d::Renderer2D renderer;


PIKA_API void testStart(pika::PikaContext pikaContext)
{
	

	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad from dll");


	gl2d::init();
	renderer.create();
	pika::setContext(pikaContext);

}


PIKA_API void testUpdate(pika::PikaContext pikaContext)
{
	gl2d::enableNecessaryGLFeatures();
	renderer.updateWindowMetrics(640, 480);
	renderer.renderRectangle({10,10, 100, 100}, Colors_Magenta);
	renderer.flush();


//	ImGui::SetAllocatorFunctions(userMalloc, userFree);


	ImGui::Begin("test");
	ImGui::End();

}