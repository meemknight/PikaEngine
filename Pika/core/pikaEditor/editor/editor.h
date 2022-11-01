#pragma once
//////////////////////////////////////////
//editor.h
//Luta Vlad(c) 2022
//https://github.com/meemknight/PikaEngine
//////////////////////////////////////////

#include <pikaImgui/pikaImgui.h>
#include <logs/logWindow.h>
#include <windowSystemm/input.h>
#include <shortcutApi/shortcutApi.h>
#include <editShortcuts/editShortcuts.h>
#include <pushNotification/pushNotification.h>
#include <containersWindow/containersWindow.h>
#include <pikaConsoleManager/pikaConsoleWindow.h>
#include <assetManagerWindow/assetManagerWindow.h>

namespace pika
{

	struct Editor
	{

		void init(pika::ShortcutManager &shortcutManager, pika::pikaImgui::ImGuiIdsManager &imguiIDManager);

		void update(const pika::Input &input, pika::ShortcutManager &shortcutManager
			,pika::LogManager &logs, pika::PushNotificationManager &pushNotificationManager,
			pika::LoadedDll &loadedDll, pika::ContainerManager &containerManager,
			pika::pikaImgui::ImGuiIdsManager &imguiIDsManager);

		void saveFlagsData();

		struct
		{
			bool hideMainWindow = 0;
		}optionsFlags;

		struct
		{
			bool logsWindow = 0;
			bool editShortcutsWindow = 0;
			bool containerManager = 0;
			bool transparentWindow = 0;
			bool consoleWindow = 0;
			bool assetManagerWindow = 0;
		}windowFlags;

		pika::LogWindow logWindow;
		pika::EditShortcutsWindow editShortcutsWindow;
		pika::ContainersWindow containersWindow;
		pika::ConsoleWindow consoleWindow;
		pika::AssetManagerWindow assetManagerWindow;

		bool lastHideWindowState = optionsFlags.hideMainWindow;

		bool shouldReloadDll = 0;
		int imguiId = 0;
	};



}