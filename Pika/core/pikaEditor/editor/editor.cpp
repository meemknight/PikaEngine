#include "editor.h"
#include <iostream>
#include "IconsForkAwesome.h"
#include "shortcutApi/shortcutApi.h"
#include <editShortcuts/editShortcuts.h>

#define DOCK_MAIN_WINDOW_SHORTCUT "Dock main window"
#define LOGS_SHORTCUT "Logs window"
#define EDIT_SHORTCUTS "Edit shortcuts window"

void pika::Editor::init(pika::ShortcutManager &shortcutManager)
{

	shortcutManager.registerShortcut(DOCK_MAIN_WINDOW_SHORTCUT, "Ctrl+Alt+D", &optionsFlags.dockMainWindow);
	shortcutManager.registerShortcut(LOGS_SHORTCUT, "Ctrl+L", &windowFlags.logsWindow);
	shortcutManager.registerShortcut(EDIT_SHORTCUTS, "", &windowFlags.editShortcutsWindow);



	logWindow.init();
	editShortcutsWindow.init();
}



void pika::Editor::update(const pika::Input &input,
	pika::ShortcutManager &shortcutManager, pika::LogManager &logs)
{

#pragma region docking space init
	ImGuiWindowFlags mainWindowFlags = ImGuiWindowFlags_MenuBar;
	if (optionsFlags.dockMainWindow)
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
	
	//todo imgui push pop id for main window
	if (ImGui::Begin(
		"Main window",
		/*p_open=*/nullptr,
		mainWindowFlags
		)
		)
	{
		//ImGui::PopStyleColor();


		if (optionsFlags.dockMainWindow)
		{
			static const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
			ImGuiID dockSpace = ImGui::GetID("MainWindowDockspace");
			ImGui::DockSpace(dockSpace, ImVec2(0.0f, 0.0f), dockspaceFlags);
		}

	#pragma region menu
		if (ImGui::BeginMenuBar())
		{

			if (ImGui::BeginMenu("Open..."))
			{

				ImGui::Text("menu text");

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options"))
			{


				ImGui::MenuItem(ICON_FK_WINDOW_RESTORE " " DOCK_MAIN_WINDOW_SHORTCUT,
					shortcutManager.getShortcut(DOCK_MAIN_WINDOW_SHORTCUT), &optionsFlags.dockMainWindow);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ICON_FK_WINDOW_MAXIMIZE " Windows"))
			{
				ImGui::MenuItem(pika::LogWindow::ICON_NAME, 
					shortcutManager.getShortcut(LOGS_SHORTCUT), &windowFlags.logsWindow);


				ImGui::EndMenu();

			}

			if (ImGui::BeginMenu(ICON_FK_COG " Settings"))
			{
				ImGui::MenuItem(pika::EditShortcutsWindow::ICON_NAME ,
					shortcutManager.getShortcut(EDIT_SHORTCUTS), &windowFlags.editShortcutsWindow);


				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	#pragma endregion

	}
	else
	{
		//ImGui::PopStyleColor();
	}
	ImGui::End();
#pragma endregion

#pragma region log window

	if (windowFlags.logsWindow)
	{
		logWindow.update(logs, windowFlags.logsWindow);
	}

	if (windowFlags.editShortcutsWindow)
	{
		editShortcutsWindow.update(shortcutManager, windowFlags.editShortcutsWindow);
	}

#pragma endregion


}
