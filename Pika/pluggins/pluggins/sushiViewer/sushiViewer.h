#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <sushi/sushi.h>


struct SushiViewer: public Container
{

	gl2d::Renderer2D renderer;
	sushi::SushyContext sushiContext;

	static ContainerStaticInfo containerInfo();

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument);

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo);

	void destruct(RequestedContainerInfo &requestedInfo);

	struct ImGuiStuff
	{
		unsigned int elementId = 0;

	}img;

};

