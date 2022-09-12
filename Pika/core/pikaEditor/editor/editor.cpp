#include "editor.h"
#include <iostream>
#include "IconsForkAwesome.h"
#include "shortcutApi/shortcutApi.h"

void pika::Editor::init(pika::ShortcutManager &shortcutManager)
{

	shortcutManager.registerShortcut("Ctrl+Alt+D", &optionsFlags.dockMainWindow);
	shortcutManager.registerShortcut("Ctrl+L", &windowFlags.logsWindow);


}



void pika::Editor::update(const pika::Input &input)
{
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


				ImGui::MenuItem(ICON_FK_WINDOW_RESTORE " Dock main window", "Ctrl+Alt+D", &optionsFlags.dockMainWindow);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows"))
			{
				ImGui::MenuItem(pika::LogWindow::ICON_NAME, "Ctrl+L", &windowFlags.logsWindow);


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



}
