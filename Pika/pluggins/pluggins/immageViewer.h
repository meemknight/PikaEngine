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
	bool uvEditor = 0;
	glm::vec4 uv = {0,1,1,0};

	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		//todo to lower
		info.extensionsSuported = {".png", ".bmp", ".psd", ".tga", ".gif", ".hdr", ".pic", ".ppm", ".pgm", ".jpg", ".jpeg"};

		info.requestImguiIds = 1;

		return info;
	}

	std::string file;

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument) override
	{

		//todo logs from containers
		//requestedInfo.

		file = commandLineArgument.to_string();

		pika::memory::setGlobalAllocatorToStandard();
		{
			texture.loadFromFile(file.c_str(), true, true);
		}
		pika::memory::setGlobalAllocator(requestedInfo.mainAllocator);

		
		immageSize = texture.GetSize();
		if (immageSize == glm::ivec2{0, 0}) { return 0; }


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


		ImGui::Text("%s; %d, %d",file.c_str(), immageSize.x, immageSize.y);
		ImGui::Checkbox("UV editor", &uvEditor);


		auto s = ImGui::GetContentRegionMax();
		
		ImGui::SetItemUsingMouseWheel();

		float wheel = ImGui::GetIO().MouseWheel;

		//todo standard out
		//todo small libraries for functionality like this

		if ((ImGui::GetIO().KeysData[ImGuiKey_LeftCtrl].Down || ImGui::GetIO().KeysData[ImGuiKey_RightCtrl].Down) && input.hasFocus)
		{
			zoom += wheel * 0.2;
		}

		zoom = std::min(zoom, 10.f);
		zoom = std::max(zoom, 0.2f);

		if (uvEditor)
		{
			
			ImGui::Separator();

			if (ImGui::BeginChild(6996, {}, false, ImGuiWindowFlags_HorizontalScrollbar))
			{

				float xsize = std::max((int)(s.x * zoom) - 10, (int)(100 * zoom));
				float aspect = 1.f; //todo

				ImGui::DragFloat4("UV", &uv[0], 0.01, 0.f, 1.f);

				ImGui::Image((void *)texture.id, {xsize,xsize / aspect},
					{uv.x, uv.y}, {uv.z, uv.w}, {1,1,1,1}, {0.2,0.2,0.2,0.9});

				ImGui::EndChild();
			}

		}
		else
		{
			ImGui::Separator();

			if (ImGui::BeginChild(6996, {}, false, ImGuiWindowFlags_HorizontalScrollbar))
			{

				float xsize = std::max((int)(s.x * zoom) - 10, (int)(100 * zoom));
				float aspect = 1.f; //todo

				ImGui::Image((void *)texture.id, {xsize,xsize / aspect},
					{0, 1}, {1, 0}, {1,1,1,1}, {0.2,0.2,0.2,0.9});

				ImGui::EndChild();
			}
		}
		

	
		ImGui::End();
		ImGui::PopID();

		return true;
	}

	void destruct(RequestedContainerInfo &requestedInfo) override
	{
		texture.cleanup();
	}
};