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


	void update(pika::Input input, float deltaTime, pika::WindowState windowState)
	{


		gl2d::enableNecessaryGLFeatures();
		renderer.updateWindowMetrics(windowState.w, windowState.h);

		renderer.renderRectangle({10, 10, 100, 100}, Colors_Magenta);

		//if (input.lMouse.pressed())
		//{
		//	std::cout << "pressed\n";
		//}
		//if (input.lMouse.released())
		//{
		//	std::cout << "released\n";
		//}

		if (input.lMouse.typed())
		{
			std::cout << "typed\n";
		}

		//std::cout << state.deltaTime << "\n";

		renderer.flush();

		//ImGui::SetAllocatorFunctions(userMalloc, userFree);

		ImGui::Begin("test");
		ImGui::End();
	}

};