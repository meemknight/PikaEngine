#include "containersWindow.h"
#include <pikaImgui/pikaImgui.h>
#include "imguiComboSearch.h"

void pika::ContainersWindow::init()
{
}

void pika::ContainersWindow::update(pika::LogManager &logManager, bool &open, pika::LoadedDll &loadedDll)
{
	//todo imgui firsttime stuff for all windows
	ImGui::PushID(EditorImguiIds::containersWindow);


	if (!ImGui::Begin(ICON_NAME, &open))
	{
		ImGui::End();
		ImGui::PopID();
		return;
	}


	// Left
	static int selected = 0;
	//{
	//	ImGui::BeginChild("left pane", ImVec2(150, 0), true);
	//	for (int i = 0; i < 100; i++)
	//	{
	//		// FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
	//		char label[128];
	//		sprintf(label, "MyObject %d", i);
	//		if (ImGui::Selectable(label, selected == i))
	//			selected = i;
	//	}
	//	ImGui::EndChild();
	//}
	//ImGui::SameLine();

	// Right
	{
		ImGui::BeginGroup();
		ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
		

		if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem(ICON_FK_PLUS_SQUARE_O " Create container"))
			{
				ImGui::Text("Available containers");
				ImGui::Separator();
				static int itemCurrent;//todo move

				//left
				ImGui::PushID(EditorImguiIds::containersWindow+1);
				ImGui::BeginGroup();
				{

					static char filter[256] = {};
					
					loadedDll.containerInfo[0];

					std::vector<std::string> containerNames;
					containerNames.reserve(loadedDll.containerInfo.size());

					for (auto &i : loadedDll.containerInfo)
					{
						containerNames.push_back(i.containerName);
					}

					auto contentSize = ImGui::GetItemRectSize();
					contentSize.y -= ImGui::GetFrameHeightWithSpacing();
					contentSize.x /= 2;

					ImGui::ListWithFilter("##list box container info", &itemCurrent, filter, sizeof(filter),
						containerNames, contentSize);


				}
				ImGui::EndGroup();
				ImGui::PopID();

				ImGui::SameLine();

				//right
				ImGui::PushID(EditorImguiIds::containersWindow + 2);
				ImGui::BeginGroup();
				{
					if (itemCurrent < loadedDll.containerInfo.size())
					{
						auto &c = loadedDll.containerInfo[itemCurrent];

						ImGui::Text("Container info: %s", c.containerName.c_str());
						ImGui::Separator();

						if (ImGui::BeginTabBar("##Tabs for container info", ImGuiTabBarFlags_Reorderable))
						{

							if (ImGui::BeginTabItem(ICON_FK_PIE_CHART " Memory"))
							{
								ImGui::NewLine();

							#pragma region total memory requirement
								size_t totalHeapMemory = c.containerStaticInfo.defaultHeapMemorySize;
								for (auto i : c.containerStaticInfo.bonusAllocators)
								{
									totalHeapMemory += i;
								}

								size_t totalMemory = totalHeapMemory + c.containerStructBaseSize;

								ImGui::Text("Total Memory requirement: %" IM_PRIu64 " (bytes)", totalMemory);
								ImGui::Text("Total Heap requirement: %" IM_PRIu64 " (bytes)", totalHeapMemory);
							#pragma endregion

								ImGui::NewLine();

								ImGui::Text("Static Memory requirement: %" IM_PRIu64 " (bytes)", c.containerStructBaseSize);
								//todo select memory unit

								ImGui::Text("Default Heap Memory requirement: %" IM_PRIu64 " (bytes)",
									c.containerStaticInfo.defaultHeapMemorySize);


								ImGui::Text("Other Heap Memory Allocators count: %" IM_PRIu64, c.containerStaticInfo.bonusAllocators.size());

								if (!c.containerStaticInfo.bonusAllocators.empty())
								{
									if (ImGui::BeginChild("##heap allocators",
										{0, 100}, true, ImGuiWindowFlags_AlwaysVerticalScrollbar))
									{

										for (auto i : c.containerStaticInfo.bonusAllocators)
										{
											ImGui::Text("%" IM_PRIu64 " (btyes)", i);
										}


									}
									ImGui::EndChild();
								}


								ImGui::EndTabItem();
							}

							if (ImGui::BeginTabItem(" Other..."))
							{
								ImGui::NewLine();

								ImGui::EndTabItem();
							}

							ImGui::EndTabBar();
						}



					}
					else
					{
						ImGui::Text("Container info:");
						ImGui::Separator();
					}

				}
				ImGui::EndGroup();
				ImGui::PopID();
				

				
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(ICON_FK_MICROCHIP " Running containers"))
			{
				ImGui::Text("Running containers");
				ImGui::Separator();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndChild();
		if (ImGui::Button("Revert")) {}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {}
		ImGui::EndGroup();
	}



	//todo container profiler will have an instance per container with imgui id and stuff.


	ImGui::End();
	ImGui::PopID();
}
