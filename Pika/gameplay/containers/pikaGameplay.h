#pragma once

#include <iostream>
#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>

struct Gameplay : public Container
{

	gl2d::Renderer2D renderer;

	void create()
	{
		renderer.create();

	}


	void update()
	{
		gl2d::enableNecessaryGLFeatures();
		renderer.updateWindowMetrics(640, 480);
		renderer.renderRectangle({10,10, 100, 100}, Colors_Magenta);
		renderer.flush();

		//ImGui::SetAllocatorFunctions(userMalloc, userFree);

		ImGui::Begin("test");
		ImGui::End();
	}

};