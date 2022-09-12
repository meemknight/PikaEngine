#include "editShortcuts.h"
#include <imgui.h>

void pika::EditShortcutsWindow::init()
{
}

void pika::EditShortcutsWindow::update(pika::ShortcutManager &shortcutManager, bool &open)
{

	//todo push pop id

	ImGui::SetNextWindowSize({400, 500});

	if (ImGui::Begin(ICON_NAME, &open,
		ImGuiWindowFlags_NoDocking | 
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse
		))
	{
		
		ImGui::Text("Edit shortcuts\n");

		//todo unique id
		if (ImGui::BeginChild(12, {}, true))
		{


			ImGui::Columns(2, 0, false);

			for (auto &shortcut : shortcutManager.registeredShortcuts)
			{

				ImGui::Text(shortcut.first.c_str());

				ImGui::NextColumn();


				char input[256] = {};
				std::strncpy(input, shortcut.second.shortcut.c_str(), sizeof(input));
				
				if (
					ImGui::InputText(("##" + shortcut.first).c_str(),
					input, sizeof(input), ImGuiInputTextFlags_EnterReturnsTrue)
					)
				{
					shortcut.second.shortcut = pika::normalizeShortcutName(input);

				}

				ImGui::NextColumn();
			}

			ImGui::Columns(1);

			ImGui::EndChild();
		}



	}
	ImGui::End();

}

