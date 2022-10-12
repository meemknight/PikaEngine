#pragma once

#include <iostream>
#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>

struct Test
{
	int *ptr = 0;
	Test() { ptr = new int(5); }
};

struct Gameplay : public Container
{

	gl2d::Renderer2D renderer;

	float *r =0;

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		info.requestImguiFbo = true; //todo this should not affect the compatibility of input recording

		//info.bonusAllocators.push_back(100);
		//info.bonusAllocators.push_back(200);
		//info.bonusAllocators.push_back(300);
		//info.bonusAllocators.push_back(400);
		//info.bonusAllocators.push_back(100);
		//info.bonusAllocators.push_back(200);
		//info.bonusAllocators.push_back(300);
		//info.bonusAllocators.push_back(400);
		//info.bonusAllocators.push_back(100);
		//info.bonusAllocators.push_back(200);
		//info.bonusAllocators.push_back(300);
		//info.bonusAllocators.push_back(400);
		//info.bonusAllocators.push_back(200);
		//info.bonusAllocators.push_back(200);

		return info;
	}


	void create(RequestedContainerInfo &requestedInfo)
	{
		renderer.create();
		//pika::initShortcutApi();
		r = new float(0);

		//void *test = new char[pika::MB(10)]; //todo let the allocator tell the engine somehow that it is out of memory
	}

	void update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
		//todo keep window on top stuff

		glClear(GL_COLOR_BUFFER_BIT);

		if (pika::shortcut(input, "Ctrl + S"))
		{
			std::cout << "save\n";
		}
		

		gl2d::enableNecessaryGLFeatures();
		renderer.updateWindowMetrics(windowState.w, windowState.h);

		*r += input.deltaTime * 4.f;

		renderer.renderRectangle({10, 10, 100, 100}, Colors_Green, {}, *r);

		//if (input.lMouse.pressed())
		//{
		//	std::cout << "pressed\n";
		//}
		//if (input.lMouse.released())
		//{
		//	std::cout << "released\n";
		//}

		//if (input.lMouse.typed())
		//{
		//	std::cout << "typed\n";
		//}

		//if (input.buttons[pika::Button::E].typed())
		//{
		//	std::cout << "e";
		//}

		//std::cout << state.deltaTime << "\n";

		renderer.flush();

		//ImGui::SetAllocatorFunctions(userMalloc, userFree);

		//ImGui::Begin("window from gameplay");
		//ImGui::Spinner("spinner", 10, 2);
		//ImGui::ProgressBar(0.4);
		//ImGui::BufferingBar("buffering bar", 0.4, {100, 5});
		//ImGui::LoadingIndicatorCircle("circle", 20, 8, 8);
		//ImGui::End();
		
		//ImGui::ShowDemoWindow();

	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release