#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <sushi/sushi.h>
#include <pikaImgui/pikaImgui.h>

struct SushiViewer: public Container
{

	gl2d::Renderer2D renderer;
	sushi::SushyContext sushiContext;

	static ContainerStaticInfo containerInfo();

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument);

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo);

	void destruct(RequestedContainerInfo &requestedInfo);

	std::vector<unsigned int> toDelete;

	struct ImGuiStuff
	{
		//set this to something to automatically change the current element
		unsigned int elementId = 0;

		bool dragging = 0;
		glm::vec2 dragBegin = {};
		glm::vec2 originalPos = {};
		pika::pikaImgui::FileSelector fileSelector{"Selected File", PIKA_RESOURCES_PATH, {".sushi"}};
		
	}img;

	void displaySushiParentElementImgui(::sushi::SushiParent &e, glm::vec4 parent, bool displayChildren);

	void displaySushiTransformImgui(::sushi::Transform &e, glm::vec4 parent, int id);

	void displaySushiBackgroundImgui(::sushi::Background &e, int id);
};

