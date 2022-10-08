#include "containersWindow.h"
#include <pikaImgui/pikaImgui.h>
#include "imguiComboSearch.h"
#include <imgui_spinner.h>
#include <validatePath.h>

void pika::ContainersWindow::init()
{
}

void pika::ContainersWindow::update(pika::LogManager &logManager, bool &open, pika::LoadedDll &loadedDll,
	pika::ContainerManager &containerManager)
{
	//todo imgui firsttime stuff for all windows
	ImGui::PushID(pikaImgui::EditorImguiIds::containersWindow);


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
				ImGui::PushID(pikaImgui::EditorImguiIds::containersWindow+1);
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
				ImGui::PushID(pikaImgui::EditorImguiIds::containersWindow + 2);
				ImGui::BeginGroup();
				{
					if (itemCurrent < loadedDll.containerInfo.size())
					{
						auto &c = loadedDll.containerInfo[itemCurrent];

						ImGui::Text("Container info: %s", c.containerName.c_str());
						ImGui::Separator();

						selectedContainerToLaunch = c.containerName;


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

								ImGui::Text("Total Memory requirement: ");
								ImGui::SameLine();
								pika::pikaImgui::displayMemorySizeValue(totalMemory);

								ImGui::Text("Total Heap requirement: ");
								ImGui::SameLine();
								pika::pikaImgui::displayMemorySizeValue(totalHeapMemory);
							#pragma endregion

								ImGui::NewLine();

								ImGui::Text("Static Memory requirement: ");
								ImGui::SameLine();
								pika::pikaImgui::displayMemorySizeValue(c.containerStructBaseSize);
								//todo select memory unit

								ImGui::Text("Default Heap Memory requirement: ");
								ImGui::SameLine();
								pika::pikaImgui::displayMemorySizeValue(c.containerStaticInfo.defaultHeapMemorySize);

								ImGui::Text("Other Heap Memory Allocators count: ");
								ImGui::SameLine();
								pika::pikaImgui::displayMemorySizeValue(c.containerStaticInfo.bonusAllocators.size());


								if (!c.containerStaticInfo.bonusAllocators.empty())
								{
									if (ImGui::BeginChild("##heap allocators",
										{0, 100}, true, ImGuiWindowFlags_AlwaysVerticalScrollbar))
									{

										for (auto i : c.containerStaticInfo.bonusAllocators)
										{
											pika::pikaImgui::displayMemorySizeValue(i);
										}


									}
									ImGui::EndChild();
								}


								ImGui::EndTabItem();
							}

							if (ImGui::BeginTabItem(ICON_FK_PLUS_SQUARE_O" Launch"))
							{
								ImGui::NewLine();

								if (!selectedContainerToLaunch.empty()
									&& ImGui::Button(ICON_FK_PLAY " Launch a default configuration"))
								{
									containerManager.createContainer(selectedContainerToLaunch, loadedDll, logManager);
								}

								ImGui::NewLine();


								if (!selectedContainerToLaunch.empty()
									&& ImGui::Button(ICON_FK_PICTURE_O " Launch a snapshot"))
								{
									//containerManager.createContainer(selectedContainerToLaunch, loadedDll, logManager);
								}

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

				pika::pikaImgui::helpMarker(
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

				ImGui::PushID(pikaImgui::EditorImguiIds::containersWindow + 3);
				ImGui::BeginGroup();
				{

					static char filter[256] = {};


					
					containerIds.reserve(containerManager.runningContainers.size());
					containerNames.reserve(containerManager.runningContainers.size());

					for (auto &i : containerManager.runningContainers)
					{
						containerIds.push_back(i.first);
						containerNames.push_back(
							std::string(i.second.flags.getStatusIcon()) + " " + i.second.baseContainerName + ": " + std::to_string(i.first));
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
				ImGui::PushID(pikaImgui::EditorImguiIds::containersWindow + 4);
				ImGui::BeginGroup();
				{
					if (itemCurrent < containerIds.size())
					{
						auto &c = containerManager.runningContainers[containerIds[itemCurrent]];

						
						ImGui::Text("Running container: %s #%u", c.baseContainerName, containerIds[itemCurrent]);
						ImGui::Separator();

					#pragma region buttons
						
						//calculate cursor pos for 3 buttons
						{
							ImGuiStyle &style = ImGui::GetStyle();
							float width = 0.0f;
							width += ImGui::CalcTextSize(ICON_FK_PAUSE).x;
							width += style.ItemSpacing.x;
							width += ImGui::CalcTextSize(ICON_FK_PAUSE).x;
							width += style.ItemSpacing.x;
							width += ImGui::CalcTextSize(ICON_FK_PAUSE).x;

							pika::pikaImgui::alignForWidth(width);
						}

						if (c.flags.running)
						{
							if (ImGui::Button(ICON_FK_PAUSE))
							{
								c.flags.running = 0;
							}

							if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
							{
								ImGui::SetTooltip("Pause container.");
							}
						}
						else
						{
							if (ImGui::Button(ICON_FK_PLAY))
							{
								c.flags.running = 1;
							}

							if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
							{
								ImGui::SetTooltip("Resume container.");
							}
						}

						ImGui::SameLine();

						bool stopped = false;

						if (ImGui::Button(ICON_FK_STOP))
						{
							//todo mabe defer here when api is made
							containerManager.destroyContainer(containerIds[itemCurrent], loadedDll, logManager);
							stopped = true;
						}
						if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
						{
							ImGui::SetTooltip("Stop container.");
						}

						ImGui::SameLine();

						if (ImGui::Button(ICON_FK_EJECT))
						{
							containerManager.forceTerminateContainer(containerIds[itemCurrent], loadedDll, logManager);
							stopped = true;
						}
						if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
						{
							ImGui::SetTooltip("Force stop container (not recomended).\nThis won't call any destructors.");
						}


					#pragma endregion

						if(!stopped)
						{

							ImGui::Text("Status: %s", c.flags.getStatusName());
							

							ImGui::Separator();

							static char snapshotName[50] = {};


							if (ImGui::Button(ICON_FK_CAMERA))
							{
								if (pika::isFileNameValid(snapshotName, sizeof(snapshotName)))
								{
									if (!containerManager.makeSnapshot(containerIds[itemCurrent], logManager, snapshotName))
									{
										logManager.log("Coultn't make snapshot", pika::logError);
									}else
									{
										logManager.log("Successfully created snapshot.");
									}
								}
								else
								{
									logManager.log("File name invalid", pika::logError);
								}

							}
							if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
							{
								ImGui::SetTooltip("Make snapshot");
							}

							ImGui::SameLine();

							//todo move

							ImGui::InputText("snapshot name", snapshotName, sizeof(snapshotName));



						}
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