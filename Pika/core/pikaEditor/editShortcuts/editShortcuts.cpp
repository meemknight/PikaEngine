

#include <pikaConfig.h>
#if !PIKA_SHOULD_REMOVE_EDITOR

#include "editShortcuts.h"
#include <imgui.h>
#include <pikaImgui/pikaImgui.h>

void pika::EditShortcutsWindow::init(pika::pikaImgui::ImGuiIdsManager &imguiIdManager)
{
	imguiId = imguiIdManager.getImguiIds(2);
	//todo save shortcuts
}

void pika::EditShortcutsWindow::update(pika::ShortcutManager &shortcutManager, bool &open)
{


	ImGui::SetNextWindowSize({400, 500});

	ImGui::PushID(imguiId);

	if (ImGui::Begin(ICON_NAME, &open,
		ImGuiWindowFlags_NoDocking | 
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse
		))
	{
		
		ImGui::Text("Edit shortcuts\n");

		if (ImGui::BeginChild(imguiId + 1, {}, true))
		{


			ImGui::Columns(2, 0, false);

			for (auto &shortcut : shortcutManager.registeredShortcuts)
			{

				ImGui::Text(shortcut.first.c_str());

				ImGui::NextColumn();


				char input[256] = {};
				std::strncpy(input, shortcut.second.shortcut.c_str(), sizeof(input));
				
				int flags = ImGuiInputTextFlags_EnterReturnsTrue;
				if (!shortcut.second.editable)
				{
					flags = flags | ImGuiInputTextFlags_ReadOnly;
				}

				if (
					ImGui::InputText(("##" + shortcut.first).c_str(),
					input, sizeof(input), flags)
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

	ImGui::PopID();

}

#endif