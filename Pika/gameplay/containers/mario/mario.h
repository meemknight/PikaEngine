#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include "marioCommon.h"

struct Mario: public Container
{

	gl2d::Renderer2D renderer;
	gl2d::Texture tiles;
	gl2d::TextureAtlasPadding atlas;
	

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		info.requestImguiFbo = true; //todo this should not affect the compatibility of input recording



		return info;
	}


	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		renderer.create();
		//gl2d::setErrorFuncCallback() //tood
		//pika::initShortcutApi();

		size_t s = 0;
		if (requestedInfo.getFileSizeBinary(PIKA_RESOURCES_PATH "/mario/1985_tiles.png", s))
		{
			void *data = new unsigned char[s];
			if (requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "/mario/1985_tiles.png", data, s))
			{
				tiles.createFromFileDataWithPixelPadding((unsigned char*)data, s, 8, true, false);

			}
			else { return 0; }

			delete[] data;
		}
		else { return 0; }


		atlas = gl2d::TextureAtlasPadding(8, 10, 8*8, 8*10);


		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
		//todo keep window on top stuff

		glClear(GL_COLOR_BUFFER_BIT);
		gl2d::enableNecessaryGLFeatures();


		renderer.updateWindowMetrics(windowState.w, windowState.h);


		renderer.renderRectangle({10, 10, 100, 100}, {}, {}, tiles, atlas.get(3, 5));


		int size = 20;
		renderer.renderRectangle({input.mouseX - size / 2, input.mouseY - size / 2, size, size},
			Colors_Turqoise, {}, 0.f);



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

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release