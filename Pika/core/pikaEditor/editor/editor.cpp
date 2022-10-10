//////////////////////////////////////////
//editor.cpp
//Luta Vlad(c) 2022
//https://github.com/meemknight/PikaEngine
//////////////////////////////////////////

#include "editor.h"
#include <iostream>
#include "IconsForkAwesome.h"
#include "shortcutApi/shortcutApi.h"
#include <editShortcuts/editShortcuts.h>

#define DOCK_MAIN_WINDOW_SHORTCUT ICON_FK_EYE_SLASH " Hide main window"
#define LOGS_SHORTCUT ICON_FK_COMMENT_O " Logs window"
#define EDIT_SHORTCUTS ICON_FK_PENCIL_SQUARE " Edit shortcuts window"
#define CONTAINERS_SHORTCUTS ICON_FK_MICROCHIP " Containers window"
#define RELOAD_DLL_SHORTCUTS ICON_FK_REFRESH " Reload dll"

void pika::Editor::init(pika::ShortcutManager &shortcutManager)
{

	shortcutManager.registerShortcut(DOCK_MAIN_WINDOW_SHORTCUT, "Ctrl+Alt+D", &optionsFlags.hideMainWindow);
	shortcutManager.registerShortcut(LOGS_SHORTCUT, "Ctrl+L", &windowFlags.logsWindow);
	shortcutManager.registerShortcut(EDIT_SHORTCUTS, "", &windowFlags.editShortcutsWindow);
	shortcutManager.registerShortcut(CONTAINERS_SHORTCUTS, "Ctrl+M", &windowFlags.containerManager);
	shortcutManager.registerShortcut(RELOAD_DLL_SHORTCUTS, "Ctrl+Alt+R", &shouldReloadDll);


	logWindow.init();
	editShortcutsWindow.init();
	containersWindow.init();
}



void pika::Editor::update(const pika::Input &input,
	pika::ShortcutManager &shortcutManager, pika::LogManager &logs, 
	pika::PushNotificationManager &pushNotificationManager, pika::LoadedDll &loadedDll
	,pika::ContainerManager &containerManager)
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
	
	
		ImGui::PushID(pika::pikaImgui::EditorImguiIds::mainEditorWindow);

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

					ImGui::EndMenu();

				}

				if (ImGui::BeginMenu(ICON_FK_COG " Settings"))
				{
					ImGui::MenuItem(pika::EditShortcutsWindow::ICON_NAME,
						shortcutManager.getShortcut(EDIT_SHORTCUTS), &windowFlags.editShortcutsWindow);

					pika::pikaImgui::displayMemorySizeToggle();

					//todo
					//ImGui::MenuItem(pika::EditShortcutsWindow::ICON_NAME,
					//	shortcutManager.getShortcut(EDIT_SHORTCUTS), &windowFlags.editShortcutsWindow);
					//transparentWindow

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
		containersWindow.update(logs, windowFlags.containerManager, loadedDll, containerManager);
	}
#pragma endregion




}
