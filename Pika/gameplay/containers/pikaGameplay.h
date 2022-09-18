#pragma once

#include <iostream>
#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>

struct Test
{
	int *ptr = 0;
	Test() { ptr = new int(5); }
};

struct Gameplay : public Container
{

	gl2d::Renderer2D renderer;

	float *r =0;


	void create()
	{
		renderer.create();
		//pika::initShortcutApi();
		r = new float(0);
	}


	void update(pika::Input input, float deltaTime, pika::WindowState windowState)
	{

		if (pika::shortcut(input, "Ctrl + S"))
		{
			std::cout << "save\n";
		}


		gl2d::enableNecessaryGLFeatures();
		renderer.updateWindowMetrics(windowState.w, windowState.h);

		*r += deltaTime * 4.f;

		renderer.renderRectangle({10, 10, 100, 100}, Colors_Blue, {}, *r);

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
		
		ImGui::ShowDemoWindow();
	}

};