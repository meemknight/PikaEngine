#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <glui/glui.h>
#include <engineLibraresSupport/engineGL2DSupport.h>

struct McDungeonsMenu : public Container
{

	gl2d::Renderer2D renderer;
	gl2d::Font font;
	gl2d::Texture logo;
	gl2d::Texture button;
	glui::RendererUi ui;

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(100);

		info.requestImguiFbo = true; 
		info.openOnApplicationStartup = true;
		
		

		return info;
	}


	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		renderer.create();
		//pika::initShortcutApi(); //todo do this in dll load

		font = pika::gl2d::loadFont(PIKA_RESOURCES_PATH "mcDungeons/CommodorePixeled.ttf", requestedInfo);
		logo = pika::gl2d::loadTexture(PIKA_RESOURCES_PATH "mcDungeons/logo.png", requestedInfo);
		button = pika::gl2d::loadTexture(PIKA_RESOURCES_PATH "mcDungeons/button.png", requestedInfo);


		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{

		glClear(GL_COLOR_BUFFER_BIT);



		renderer.updateWindowMetrics(windowState.w, windowState.h);

		
		ui.Begin(69);
		{
			
			ui.Texture(1, logo);

			if (ui.Button("Start game", Colors_White, button))
			{
				requestedInfo.createContainer("McDungeonsGameplay");
				return 0;
			}

			//ui.BeginMenu("Settings", Colors_White, button);
			//{
			//	if (ui.Button("Adaptive rezolution", Colors_White, button))
			//	{
			//
			//	}
			//}
			//ui.EndMenu();

		}
		ui.End();


		ui.renderFrame(renderer, font, {input.mouseX, input.mouseY},
			input.lMouse.pressed(), input.lMouse.held(), input.lMouse.released(),
			input.buttons[pika::Button::Escape].released(), input.typedInput, input.deltaTime);

		renderer.flush();

		

		return true;
	}

	void destruct()
	{

		renderer.clear();
		font.texture.cleanup();
		logo.cleanup();
		button.cleanup();

	}

};
