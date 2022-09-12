#pragma once

#include <iostream>
#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>

struct Gameplay : public Container
{

	gl2d::Renderer2D renderer;

	void create()
	{
		renderer.create();

	}

	float r = 0;

	void update(pika::Input input, float deltaTime, pika::WindowState windowState)
	{

		if (pika::shortcut(input, "Ctrl + S"))
		{
			std::cout << "save\n";
		}


		gl2d::enableNecessaryGLFeatures();
		renderer.updateWindowMetrics(windowState.w, windowState.h);

		r += deltaTime * 3.f;

		renderer.renderRectangle({10, 10, 100, 100}, Colors_Magenta, {}, r);

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

		//ImGui::Begin("window from gameplay");
		//ImGui::End();
		
		//ImGui::ShowDemoWindow();
	}

};