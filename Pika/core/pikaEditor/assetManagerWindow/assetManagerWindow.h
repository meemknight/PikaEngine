#pragma once

#include "pikaConfig.h"
#if !PIKA_SHOULD_REMOVE_EDITOR

#include <logs/log.h>
#include <IconsForkAwesome.h>
#include <imgui.h>
#include <pikaImgui/pikaImgui.h>
#include <filesystem>
#include <string>
#include <containerManager/containerManager.h>

namespace pika
{


	struct AssetManagerWindow
	{

		void init(pika::pikaImgui::ImGuiIdsManager &idManager);

		void update(bool &open, ContainerManager &containerManager, LoadedDll &currentDll,
			pika::LogManager &logManager, pika::pikaImgui::ImGuiIdsManager &imguiIDsManager, ConsoleWindow *consoleWindow);

		static constexpr char *ICON = ICON_FK_FILES_O;
		static constexpr char *NAME = "Asset manager";
		static constexpr char *ICON_NAME = ICON_FK_FILES_O " Asset manager";

		int imguiId = 0;
		char searchText[100] = {};

		std::filesystem::path currentPath = PIKA_RESOURCES_PATH;
	};

}

#endif