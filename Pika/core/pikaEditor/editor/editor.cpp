#include "editor.h"

void pika::Editor::init()
{
}

void pika::Editor::update()
{
	ImGuiWindowFlags mainWindowFlags = ImGuiWindowFlags_MenuBar;
	if (optionsFlags.dockMainWindow)
	{
		mainWindowFlags = ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoTitleBar;

		ImVec2 vWindowSize = ImGui::GetMainViewport()->Size;
		ImVec2 vPos0 = ImGui::GetMainViewport()->Pos;
		ImGui::SetNextWindowPos(ImVec2((float)vPos0.x, (float)vPos0.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2((float)vWindowSize.x, (float)vWindowSize.y), 0);
	}

	//todo imgui push pop id for main window
	if (ImGui::Begin(
		"Main window",
		/*p_open=*/nullptr,
		mainWindowFlags
		)
		)
	{

		if (optionsFlags.dockMainWindow)
		{
			static const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
			ImGuiID dockSpace = ImGui::GetID("MainWindowDockspace");
			ImGui::DockSpace(dockSpace, ImVec2(0.0f, 0.0f), dockspaceFlags);
		}


		if (ImGui::BeginMenuBar())
		{

			if (ImGui::BeginMenu("Open..."))
			{

				ImGui::Text("menu text");

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options"))
			{

				ImGui::Checkbox("Dock main window", &optionsFlags.dockMainWindow);

				ImGui::EndMenu();
			}


			ImGui::EndMenuBar();
		}

	}
	ImGui::End();



}
