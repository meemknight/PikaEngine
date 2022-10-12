#pragma once

#include <iostream>
#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <fstream>
#include <globalAllocator/globalAllocator.h>

struct ImmageViewer: public Container
{

	gl2d::Texture texture;

	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(20);

		//todo imgui ids

		return info;
	}

	void create(RequestedContainerInfo &requestedInfo)
	{
		


		pika::memory::setGlobalAllocatorToStandard();
		{
			texture.loadFromFile(PIKA_RESOURCES_PATH "map.png", true, true);
		}
		pika::memory::setGlobalAllocator(requestedInfo.mainAllocator);

		//texture.cleanup();
		//GLuint b = 0;
		//glGenBuffers(1, &b);

		//malloc(100);
		//new int[100];
	}

	void update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{

		
		if(!ImGui::Begin("immage viewer"))
		{
			ImGui::End();
			return;
		}

		float xsize = 400.f;
		float aspect = 1.f;

		ImGui::Image((void *)texture.id, {xsize,xsize / aspect},
			{0, 1}, {1, 0});


		ImGui::End();
	}

};