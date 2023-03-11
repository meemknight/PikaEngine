#pragma once

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

	float *r = 0;

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


	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		renderer.create();
		//pika::initShortcutApi();
		r = new float;

		//void *test = new char[pika::MB(10)]; //todo let the allocator tell the engine somehow that it is out of memory
		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
		//todo keep window on top stuff

		glClear(GL_COLOR_BUFFER_BIT);

		//if (pika::shortcut(input, "Ctrl + S"))
		//{
		//	requestedInfo.consoleWrite("save\n");
		//}
		
		//if (input.buttons[pika::Button::S].released())
		//{
		//	requestedInfo.consoleWrite("save\n");
		//}

		if (input.buttons[pika::Button::G].released())
		{
			requestedInfo.createContainer("Gameplay");
		}

		requestedInfo.consoleWrite(input.typedInput);



		if (input.buttons[pika::Button::P].held())
		{
			requestedInfo.consoleWrite(std::to_string(input.deltaTime).c_str());
		}

		gl2d::enableNecessaryGLFeatures();
		renderer.updateWindowMetrics(windowState.w, windowState.h);

		*r += input.deltaTime * 4.f;

		if (input.hasFocus)
		{
			renderer.renderRectangle({10, 10, 100, 100}, Colors_Green, {}, *r);
		}
		else
		{
			renderer.renderRectangle({10, 10, 100, 100}, Colors_Blue, {}, *r);
		}
		
		int size = 11;
		renderer.renderRectangle({input.mouseX-size/2, input.mouseY-size/2, size, size},
			Colors_Red, {}, 0.f);


		//requestedInfo.consoleWrite((std::string("Mouse: ") + std::to_string(input.mouseX) + " " +
		//	std::to_string(input.mouseY) + "\n").c_str());


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

		return true;
	}

	//optional
	void destruct()
	{

	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release