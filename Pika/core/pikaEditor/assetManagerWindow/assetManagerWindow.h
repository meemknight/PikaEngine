#pragma once
#if !PIKA_SHOULD_REMOVE_IMGUI

#include <logs/log.h>
#include <IconsForkAwesome.h>
#include <imgui.h>
#include <pikaImgui/pikaImgui.h>
#include <filesystem>
#include <string>

namespace pika
{


	struct AssetManagerWindow
	{

		void init(pika::pikaImgui::ImGuiIdsManager &idManager);

		void update(bool &open);

		static constexpr char *ICON = ICON_FK_FILES_O;
		static constexpr char *NAME = "Asset manager";
		static constexpr char *ICON_NAME = ICON_FK_FILES_O " Asset manager";

		int imguiId = 0;

		std::filesystem::path currentPath = PIKA_RESOURCES_PATH;
	};

}

#endif