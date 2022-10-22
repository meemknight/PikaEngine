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
	float zoom = 1.f;

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

		
	}

	void update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
		
		
		if(!ImGui::Begin("Immage title"))
		{
			ImGui::End();
			return;
		}

		ImGui::Text("Immage title; %d, %d", 100, 100);

		auto s = ImGui::GetContentRegionMax();
		
		ImGui::SetItemUsingMouseWheel();

		float wheel = ImGui::GetIO().MouseWheel;

		//todo standard out

		if (ImGui::GetIO().KeysData[ImGuiKey_LeftCtrl].Down || ImGui::GetIO().KeysData[ImGuiKey_RightCtrl].Down)
		{
			zoom += wheel * 0.2;
		}

		zoom = std::min(zoom, 10.f);
		zoom = std::max(zoom, 0.2f);

		if (ImGui::BeginChild(6996, {}, false, ImGuiWindowFlags_HorizontalScrollbar))
		{

			float xsize = std::max((int)(s.x*zoom) - 10, (int)(100*zoom));
			float aspect = 1.f;

			ImGui::Image((void *)texture.id, {xsize,xsize / aspect},
				{0, 1}, {1, 0});

			ImGui::EndChild();
		}


		ImGui::End();
	}

};