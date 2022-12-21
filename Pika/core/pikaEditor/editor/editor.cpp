//////////////////////////////////////////
//editor.cpp
//Luta Vlad(c) 2022
//https://github.com/meemknight/PikaEngine
//////////////////////////////////////////

#if !PIKA_SHOULD_REMOVE_EDITOR

#include "editor.h"
#include <iostream>
#include "IconsForkAwesome.h"
#include "shortcutApi/shortcutApi.h"
#include <editShortcuts/editShortcuts.h>
#include <safeSave.h>



#define DOCK_MAIN_WINDOW_SHORTCUT ICON_FK_EYE_SLASH " Hide main window"
#define LOGS_SHORTCUT ICON_FK_COMMENT_O " Logs window"
#define EDIT_SHORTCUTS ICON_FK_PENCIL_SQUARE " Edit shortcuts window"
#define CONTAINERS_SHORTCUTS ICON_FK_MICROCHIP " Containers window"
#define RELOAD_DLL_SHORTCUTS ICON_FK_REFRESH " Reload dll"
#define TRANSPARENT_EDITOR_WINDOW ICON_FK_EYE " Transparent Editor window"
#define CONSOLE_WINDOW ICON_FK_TERMINAL " Console window"
#define ASSET_MANAGER_WINDOW ICON_FK_FILES_O " Asset manager"
#define OPENGL_ERRORS_WINDOW ICON_FK_EXCLAMATION_TRIANGLE " opengl errors"

#if !PIKA_SHOULD_REMOVE_EDITOR

void pika::Editor::init(pika::ShortcutManager &shortcutManager, pika::pikaImgui::ImGuiIdsManager &imguiIDManager)
{

	shortcutManager.registerShortcut(DOCK_MAIN_WINDOW_SHORTCUT, "Ctrl+Alt+D", &optionsFlags.hideMainWindow);
	shortcutManager.registerShortcut(LOGS_SHORTCUT, "Ctrl+L", &windowFlags.logsWindow);
	shortcutManager.registerShortcut(EDIT_SHORTCUTS, "", &windowFlags.editShortcutsWindow);
	shortcutManager.registerShortcut(CONTAINERS_SHORTCUTS, "Ctrl+M", &windowFlags.containerManager);
	shortcutManager.registerShortcut(RELOAD_DLL_SHORTCUTS, "Ctrl+Alt+R", &shouldReloadDll);
	shortcutManager.registerShortcut(TRANSPARENT_EDITOR_WINDOW, "Ctrl+Alt+T", &windowFlags.transparentWindow);
	shortcutManager.registerShortcut(CONSOLE_WINDOW, "Ctrl+C", &windowFlags.consoleWindow);
	shortcutManager.registerShortcut(ASSET_MANAGER_WINDOW, "Ctrl+Alt+A", &windowFlags.assetManagerWindow);
	shortcutManager.registerShortcut(OPENGL_ERRORS_WINDOW, "Ctrl+Alt+O", &windowFlags.openglErrorsWindow);

	imguiId = imguiIDManager.getImguiIds(1);

	logWindow.init(imguiIDManager);
	editShortcutsWindow.init(imguiIDManager);
	containersWindow.init(imguiIDManager);
	consoleWindow.init(imguiIDManager);
	assetManagerWindow.init(imguiIDManager);
	openglLogsWindow.init(imguiIDManager);

	if (sfs::safeLoad(&optionsFlags, sizeof(optionsFlags), PIKA_ENGINE_SAVES_PATH "options", false) != sfs::noError)
	{
		optionsFlags = {};
	}

	if (sfs::safeLoad(&windowFlags, sizeof(windowFlags), PIKA_ENGINE_SAVES_PATH "window", false) != sfs::noError)
	{
		windowFlags = {};
	}
	

}



void pika::Editor::update(const pika::Input &input,
	pika::ShortcutManager &shortcutManager, pika::LogManager &logs, 
	pika::PushNotificationManager &pushNotificationManager, pika::LoadedDll &loadedDll
	,pika::ContainerManager &containerManager, pika::pikaImgui::ImGuiIdsManager &imguiIDsManager)
{

#pragma region push notification if hide window

	if (lastHideWindowState == 0 && optionsFlags.hideMainWindow)
	{
		std::string message = "Press ";
		message += shortcutManager.getShortcut(DOCK_MAIN_WINDOW_SHORTCUT);
		message += " to restore the main window.";

		pushNotificationManager.pushNotification(message.c_str());
	}

	lastHideWindowState = optionsFlags.hideMainWindow;
#pragma endregion


	if (!optionsFlags.hideMainWindow)
	{

#pragma region docking space init
		ImGuiWindowFlags mainWindowFlags = ImGuiWindowFlags_MenuBar;
		//if (optionsFlags.hideMainWindow)
		{
			mainWindowFlags = ImGuiWindowFlags_MenuBar |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_NoTitleBar;

			ImVec2 vWindowSize = ImGui::GetMainViewport()->Size;
			ImVec2 vPos0 = ImGui::GetMainViewport()->Pos;
			ImGui::SetNextWindowPos(ImVec2((float)vPos0.x, (float)vPos0.y), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2((float)vWindowSize.x, (float)vWindowSize.y), 0);
		}
#pragma endregion

#pragma region main editor window

		//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.3f, 1.0f));
	
	
		ImGui::PushID(imguiId);

		ImGui::SetNextWindowBgAlpha(0);

		if (ImGui::Begin(
			"Main window",
			/*p_open=*/nullptr,
			mainWindowFlags
			)
			)
		{


			//if (optionsFlags.dockMainWindow)
			{
				static const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
				ImGuiID dockSpace = ImGui::GetID("MainWindowDockspace");
				ImGui::DockSpace(dockSpace, ImVec2(0.0f, 0.0f), dockspaceFlags);
			}

		#pragma region menu
			if (ImGui::BeginMenuBar())
			{

				if (ImGui::BeginMenu(ICON_FK_COGS " Engine"))
				{
					//todo submit tasks to the engine (usefull for this and also in gameplay)

					if (ImGui::MenuItem(ICON_FK_REFRESH " Reload dll",
						shortcutManager.getShortcut(RELOAD_DLL_SHORTCUTS), nullptr))
					{
						shouldReloadDll = true;
					}


					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Options"))
				{


					ImGui::MenuItem(DOCK_MAIN_WINDOW_SHORTCUT,
						shortcutManager.getShortcut(DOCK_MAIN_WINDOW_SHORTCUT), &optionsFlags.hideMainWindow);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu(ICON_FK_WINDOW_MAXIMIZE " Windows"))
				{
					ImGui::MenuItem(pika::LogWindow::ICON_NAME,
						shortcutManager.getShortcut(LOGS_SHORTCUT), &windowFlags.logsWindow);

					ImGui::MenuItem(pika::ContainersWindow::ICON_NAME,
						shortcutManager.getShortcut(CONTAINERS_SHORTCUTS), &windowFlags.containerManager);

					ImGui::MenuItem(pika::ConsoleWindow::ICON_NAME,
						shortcutManager.getShortcut(CONSOLE_WINDOW), &windowFlags.consoleWindow);

					ImGui::MenuItem(pika::AssetManagerWindow::ICON_NAME,
						shortcutManager.getShortcut(ASSET_MANAGER_WINDOW), &windowFlags.assetManagerWindow);

					ImGui::MenuItem(pika::OpenglLogsWindow::ICON_NAME,
						shortcutManager.getShortcut(OPENGL_ERRORS_WINDOW), &windowFlags.openglErrorsWindow);

					ImGui::EndMenu();

				}

				if (ImGui::BeginMenu(ICON_FK_COG " Settings"))
				{
					ImGui::MenuItem(pika::EditShortcutsWindow::ICON_NAME,
						shortcutManager.getShortcut(EDIT_SHORTCUTS), &windowFlags.editShortcutsWindow);

					pika::pikaImgui::displayMemorySizeToggle();

					ImGui::MenuItem(TRANSPARENT_EDITOR_WINDOW,
						shortcutManager.getShortcut(TRANSPARENT_EDITOR_WINDOW), 
						&windowFlags.transparentWindow);

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}
		#pragma endregion

		}
		ImGui::End();

		ImGui::PopID();

	#pragma endregion

	}

	if (windowFlags.transparentWindow)
	{
		ImGuiStyle &style = ::ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg].w = 0.f;
	}
	else
	{
		ImGuiStyle &style = ::ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg].w = 1.f;
	}

#pragma region log window
	if (windowFlags.logsWindow)
	{
		logWindow.update(logs, windowFlags.logsWindow);
	}
#pragma endregion

#pragma region shortcuts window
	if (windowFlags.editShortcutsWindow)
	{
		editShortcutsWindow.update(shortcutManager, windowFlags.editShortcutsWindow);
	}
#pragma endregion

#pragma region containers window
	if (windowFlags.containerManager)
	{
		containersWindow.update(logs, windowFlags.containerManager, 
			loadedDll, containerManager, imguiIDsManager, &consoleWindow);
	}
#pragma endregion

#pragma region console
	if (windowFlags.consoleWindow)
	{
		consoleWindow.update(logs, windowFlags.consoleWindow);
	}
#pragma endregion

#pragma region asset manager window

	if (windowFlags.assetManagerWindow)
	{
		assetManagerWindow.update(windowFlags.assetManagerWindow);
	}

#pragma endregion

#pragma region opengl logs

	if (windowFlags.openglErrorsWindow)
	{
		openglLogsWindow.update(windowFlags.openglErrorsWindow);
	}

#pragma endregion


}

void pika::Editor::saveFlagsData()
{

	sfs::safeSave(&optionsFlags, sizeof(optionsFlags), PIKA_ENGINE_SAVES_PATH "options", false);
	sfs::safeSave(&windowFlags, sizeof(windowFlags), PIKA_ENGINE_SAVES_PATH "window", false);


}

#endif

#endif
