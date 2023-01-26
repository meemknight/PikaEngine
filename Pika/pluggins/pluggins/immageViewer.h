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
	glm::ivec2 immageSize;

	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(30);

		//todo to lower
		info.extensionsSuported = {".png", ".bmp", ".psd", ".tga", ".gif", ".hdr", ".pic", ".ppm", ".pgm", ".jpg", ".jpeg"};

		info.requestImguiIds = 1;

		return info;
	}

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument) override
	{
		//todo logs from containers
		//requestedInfo.

		std::string file = commandLineArgument.to_string();

		//pika::memory::setGlobalAllocatorToStandard();
		//{
		//	texture.loadFromFile(PIKA_RESOURCES_PATH "map.png", true, true);
		//}
		//pika::memory::setGlobalAllocator(requestedInfo.mainAllocator);


		size_t size = 0;
		if (!requestedInfo.getFileSizeBinary(file.c_str(), size))
		{
			return 0; //todo
		}
		
		unsigned char *buffer = new unsigned char[size];
		
		if (!requestedInfo.readEntireFileBinary(file.c_str(), buffer, size))
		{
			delete[] buffer;
			return 0; //todo
		}
		
		texture.createFromFileData(buffer, size, true, true);
		
		immageSize = texture.GetSize();


		delete[] buffer;

		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo) override
	{

		//todo deffer
		ImGui::PushID(requestedInfo.requestedImguiIds);

		ImGui::SetNextWindowSize(ImVec2{(float)immageSize.x, (float)immageSize.y}, ImGuiCond_Appearing);
		
		bool open = 1;
		if(!ImGui::Begin( std::string( "Immage viewer##" + std::to_string(requestedInfo.requestedImguiIds)).c_str() , &open))
		{
			if (!open)
			{
				ImGui::End();
				ImGui::PopID();
				return false;
			}

			ImGui::End();
			ImGui::PopID();
			return true;
		}

		if (!open)
		{
			ImGui::End();
			ImGui::PopID();
			return false;
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
			float aspect = 1.f; //todo

			ImGui::Image((void *)texture.id, {xsize,xsize / aspect},
				{0, 1}, {1, 0});

			ImGui::EndChild();
		}


		ImGui::End();
		ImGui::PopID();

		return true;
	}

};