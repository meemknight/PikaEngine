#include "containersWindow.h"
#include <pikaImgui/pikaImgui.h>
#include "imguiComboSearch.h"
#include <imgui_spinner.h>

void pika::ContainersWindow::init()
{
}

void pika::ContainersWindow::update(pika::LogManager &logManager, bool &open, pika::LoadedDll &loadedDll,
	pika::ContainerManager &containerManager)
{
	//todo imgui firsttime stuff for all windows
	ImGui::PushID(EditorImguiIds::containersWindow);


	if (!ImGui::Begin(ICON_NAME, &open))
	{
		ImGui::End();
		ImGui::PopID();
		return;
	}


	static int selected = 0;
	
	std::string selectedContainerToLaunch = "";

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

						selectedContainerToLaunch = c.containerName;

						if (!selectedContainerToLaunch.empty()
							&& ImGui::Button(ICON_FK_PLUS_SQUARE_O " Launch"))
						{
							containerManager.createContainer(selectedContainerToLaunch, loadedDll, logManager);
						}



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

							if (ImGui::BeginTabItem("Other..."))
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

				ImGui::SameLine();

				pika::helpMarker(
					ICON_FK_BOLT ": Container is running;\n"
					ICON_FK_PAUSE_CIRCLE_O ": Container is on pause;\n"
					ICON_FK_VIDEO_CAMERA ": Container is being recorded;\n"
					ICON_FK_REPEAT ": Container is on input playback."
				);

				ImGui::Separator();
			
				static int itemCurrent;//todo move

				//left
				std::vector<pika::containerId_t> containerIds;
				std::vector<std::string> containerNames;

				ImGui::PushID(EditorImguiIds::containersWindow + 3);
				ImGui::BeginGroup();
				{

					static char filter[256] = {};


					
					containerIds.reserve(containerManager.runningContainers.size());
					containerNames.reserve(containerManager.runningContainers.size());

					for (auto &i : containerManager.runningContainers)
					{
						containerIds.push_back(i.first);
						containerNames.push_back(
							ICON_FK_BOLT " " + i.second.baseContainerName + ": " + std::to_string(i.first));
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
				ImGui::PushID(EditorImguiIds::containersWindow + 4);
				ImGui::BeginGroup();
				{
					if (itemCurrent < containerIds.size())
					{
						auto &c = containerManager.runningContainers[containerIds[itemCurrent]];

						
						ImGui::Text("Running container: %s #%u", c.baseContainerName.c_str(), containerIds[itemCurrent]);
						ImGui::Separator();


					}
					else
					{
						ImGui::Text("Running container:");
						ImGui::Separator();
					}

				}
				ImGui::EndGroup();
				ImGui::PopID();
				
				
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndChild();
		
		//you can add buttons here
		
		ImGui::EndGroup();
	}



	//todo container profiler will have an instance per container with imgui id and stuff.


	ImGui::End();
	ImGui::PopID();
}

//todo options hide show push notidications also engine push notifications that are forced