#include "pikaConfig.h"

#if !PIKA_SHOULD_REMOVE_IMGUI

#include "containersWindow.h"
#include <pikaImgui/pikaImgui.h>
#include "imguiComboSearch.h"
#include <imgui_spinner.h>
#include <validatePath.h>

void pika::ContainersWindow::init(pika::pikaImgui::ImGuiIdsManager &imguiIdsManager)
{
	imguiIds = imguiIdsManager.getImguiIds(10);
}

void pika::ContainersWindow::update(pika::LogManager &logManager, bool &open, pika::LoadedDll &loadedDll,
	pika::ContainerManager &containerManager, pika::pikaImgui::ImGuiIdsManager &imguiIdsManager, pika::ConsoleWindow *consoleWindow)
{
	ImGui::PushID(imguiIds);


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

		if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_Reorderable))
		{
			if (ImGui::BeginTabItem(ICON_FK_PLUS_SQUARE_O " Create container"))
			{
				ImGui::Text("Available containers");
				ImGui::Separator();

				//left
				ImGui::PushID(imguiIds +1);
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

					ImGui::ListWithFilter("##list box container info", &itemCurrentAvailableCOntainers, filter, sizeof(filter),
						containerNames, contentSize);


				}
				ImGui::EndGroup();
				ImGui::PopID();

				ImGui::SameLine();

				//right
				ImGui::PushID(imguiIds + 2);
				ImGui::BeginGroup();
				{
					if (itemCurrentAvailableCOntainers < loadedDll.containerInfo.size())
					{
						auto &c = loadedDll.containerInfo[itemCurrentAvailableCOntainers];

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
									if (createAtSpecificMemoryRegion)
									{
										containerManager.createContainer(selectedContainerToLaunch, loadedDll,
											logManager, imguiIdsManager, consoleWindow, pika::TB(1));
									}
									else
									{
										containerManager.createContainer(selectedContainerToLaunch, loadedDll, logManager,
											imguiIdsManager, consoleWindow);
									}
								}

								ImGui::Checkbox("allocate at specific memory region", &createAtSpecificMemoryRegion);


								//ImGui::NewLine();

								{
									static int currentSelectedSnapshot = 0;
									auto snapshots = pika::getAvailableSnapshotsAnyMemoryPosition(c);

									if (!selectedContainerToLaunch.empty()
										&& ImGui::Button(ICON_FK_PICTURE_O " Launch a snapshot"))
									{
										auto s = snapshots[currentSelectedSnapshot];

										auto memPos = getSnapshotMemoryPosition(s.c_str());

										if (memPos == nullptr)
										{
											logManager.log("Failes to get snapshot info", pika::logError);
										}
										else
										{

											auto c = containerManager.createContainer(
												selectedContainerToLaunch, loadedDll, logManager,
												imguiIdsManager, consoleWindow, (size_t)memPos);

											//no need to log error since create container does that
											if (c != 0)
											{
												if (!containerManager.setSnapshotToContainer(c,
													s.c_str(), logManager, imguiIdsManager))
												{
													containerManager.destroyContainer(c, loadedDll, logManager);
												}
											}


										}

										
									
									
									}


									auto contentSize = ImGui::GetItemRectSize();
									contentSize.y -= ImGui::GetFrameHeightWithSpacing();
									//contentSize.x /= 2;

									ImGui::ListWithFilter("##list box snapshots", &currentSelectedSnapshot,
										filterSnapshots, sizeof(filterSnapshots),
										snapshots, contentSize);
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
			
				//left
				std::vector<pika::containerId_t> containerIds;
				std::vector<std::string> containerNames;

				ImGui::PushID(imguiIds + 3);
				ImGui::BeginGroup();
				{
					
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

					ImGui::ListWithFilter("##list box container info", &itemCurrentCreatedContainers, 
						filterContainerInfo, sizeof(filterContainerInfo),
						containerNames, contentSize);


				}
				ImGui::EndGroup();
				ImGui::PopID();

				ImGui::SameLine();

				//right
				ImGui::PushID(imguiIds + 4);
				ImGui::BeginGroup();
				{
					if (itemCurrentCreatedContainers < containerIds.size())
					{
						auto &c = containerManager.runningContainers[containerIds[itemCurrentCreatedContainers]];

						
						ImGui::Text("Running container: %s #%u", c.baseContainerName, containerIds[itemCurrentCreatedContainers]);
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

						if (c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_RUNNING)
						{
							if (ImGui::Button(ICON_FK_PAUSE))
							{
								c.flags.status = pika::RuntimeContainer::FLAGS::STATUS_PAUSE;
							}

							if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
							{
								ImGui::SetTooltip("Pause container.");
							}
						}
						else if (c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_PAUSE)
						{
							if (ImGui::Button(ICON_FK_PLAY))
							{
								c.flags.status = pika::RuntimeContainer::FLAGS::STATUS_RUNNING;
							}

							if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
							{
								ImGui::SetTooltip("Resume container.");
							}
						}
						else if (c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_BEING_RECORDED)
						{
							ImGui::BeginDisabled();
							if (ImGui::Button(ICON_FK_PAUSE))
							{
							}
							ImGui::EndDisabled();

							if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
							{
								ImGui::SetTooltip("Can't pause container while it is recorded."); //todo implement
							}
						}
						else if (c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_BEING_PLAYBACK)
						{
							if (ImGui::Button(ICON_FK_REPEAT))
							{
								c.flags.status = pika::RuntimeContainer::FLAGS::STATUS_RUNNING;
							}

							if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
							{
								ImGui::SetTooltip("Stop playback");
							}
						}


						ImGui::SameLine();

						bool stopped = false;

						if (ImGui::Button(ICON_FK_STOP))
						{
							//todo mabe defer here when api is made
							containerManager.destroyContainer(containerIds[itemCurrentCreatedContainers], loadedDll, logManager);
							stopped = true;
						}
						if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
						{
							ImGui::SetTooltip("Stop container.");
						}

						ImGui::SameLine();

						if (ImGui::Button(ICON_FK_EJECT))
						{
							containerManager.forceTerminateContainer(containerIds[itemCurrentCreatedContainers], loadedDll, logManager);
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

							if (ImGui::BeginTabBar("##Tabs for play and record", ImGuiTabBarFlags_Reorderable))
							{
								if (ImGui::BeginTabItem(ICON_FK_CAMERA " Snapshot"))
								{
									//snapshot button
									{
										if (ImGui::Button(ICON_FK_CAMERA))
										{
											if (pika::isFileNameValid(snapshotName, sizeof(snapshotName)))
											{
												if (!containerManager.makeSnapshot(containerIds[itemCurrentCreatedContainers], logManager, snapshotName))
												{
													logManager.log("Coultn't make snapshot", pika::logError);
												}
												else
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
										ImGui::InputText("snapshot name", snapshotName, sizeof(snapshotName));
									}

									ImGui::NewLine();
									ImGui::Separator();

								#pragma region snapshots
									{

										auto snapshots = pika::getAvailableSnapshots(
											containerManager.runningContainers[containerIds[itemCurrentCreatedContainers]]);

										auto contentSize = ImGui::GetItemRectSize();
										contentSize.y -= ImGui::GetFrameHeightWithSpacing();
										contentSize.x /= 2;

										ImGui::ListWithFilter("##list box snapshots", &currentSelectedSnapshot,
											filterSnapshots, sizeof(filterSnapshots),
											snapshots, contentSize);

										ImGui::SameLine();

										if (snapshots.size() == 0 || currentSelectedSnapshot >= snapshots.size())
										{
											ImGui::BeginDisabled(true);
										}
										else
										{
											ImGui::BeginDisabled(false);
										}

										if (ImGui::Button(ICON_FK_PLAY "##play snapshot"))
										{
											if (!containerManager.setSnapshotToContainer(
												containerIds[itemCurrentCreatedContainers],
												snapshots[currentSelectedSnapshot].c_str(), logManager, imguiIdsManager
												))
											{
												logManager.log("Failed to assign snapshot", pika::logError);
											}
										}

										ImGui::EndDisabled();

										if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
										{
											ImGui::SetTooltip("Play this snapshot to this container");
										}

									}
								#pragma endregion

									ImGui::EndTabItem();

								}

								if (ImGui::BeginTabItem(ICON_FK_VIDEO_CAMERA " Record"))
								{
									//recording
									{
										if (c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_RUNNING)
										{
											if (ImGui::Button(ICON_FK_VIDEO_CAMERA))
											{
												if (pika::isFileNameValid(recordingName, sizeof(recordingName)))
												{
													containerManager.startRecordingContainer
													(containerIds[itemCurrentCreatedContainers], logManager, recordingName);
												}
												else
												{
													logManager.log("File name invalid", pika::logError);
												}

											}
											if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
											{
												ImGui::SetTooltip("start recording");
											}
										}
										else if (c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_BEING_RECORDED)
										{
											if (ImGui::Button(ICON_FK_STOP_CIRCLE))
											{
												containerManager.stopRecordingContainer
												(containerIds[itemCurrentCreatedContainers], logManager);
											}
											if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
											{
												ImGui::SetTooltip("stop recording");
											}
										}
										else if (c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_PAUSE)
										{
											ImGui::BeginDisabled(1);
											if (ImGui::Button(ICON_FK_VIDEO_CAMERA))
											{

											}
											ImGui::EndDisabled();
											if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
											{
												ImGui::SetTooltip("Can't record while container is paused");
											}
										}
										else if (c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_BEING_PLAYBACK)
										{
											ImGui::BeginDisabled(1);
											if (ImGui::Button(ICON_FK_VIDEO_CAMERA))
											{

											}
											ImGui::EndDisabled();
											if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
											{
												ImGui::SetTooltip("Can't record while container is on playback");
											}
										}

										ImGui::BeginDisabled(c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_BEING_RECORDED);
										ImGui::SameLine();
										ImGui::InputText("recording name", recordingName, sizeof(recordingName));
										ImGui::EndDisabled();

									}


									ImGui::NewLine();
									ImGui::Separator();


								#pragma region recordings

									auto recordings = pika::getAvailableRecordings(
										containerManager.runningContainers[containerIds[itemCurrentCreatedContainers]]);

									auto contentSize = ImGui::GetItemRectSize();
									contentSize.y -= ImGui::GetFrameHeightWithSpacing();
									contentSize.x /= 2;

									ImGui::ListWithFilter("##list box recordings", &currentSelectedRecording,
										filterSnapshots, sizeof(filterSnapshots),
										recordings, contentSize);

									ImGui::SameLine();

									if (recordings.size() == 0 || currentSelectedRecording >= recordings.size()
										|| c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_BEING_RECORDED)
									{
										ImGui::BeginDisabled(true);
									}
									else
									{
										ImGui::BeginDisabled(false);
									}

									if (ImGui::Button(ICON_FK_PLAY "##play recording"))
									{
										if (!containerManager.setRecordingToContainer(
											containerIds[itemCurrentCreatedContainers],
											recordings[currentSelectedSnapshot].c_str(), logManager, imguiIdsManager
											))
										{
											logManager.log("Failed to assign recording", pika::logError);
										}
									}

									ImGui::EndDisabled();
														
									if (c.flags.status == pika::RuntimeContainer::FLAGS::STATUS_BEING_RECORDED)
									{
										if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
										{
											ImGui::SetTooltip("Can't play a recording because the container is being recorded at the moment");
										}
									}
									else
									{
										if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
										{
											ImGui::SetTooltip("Play this recording to this container");
										}
									}
									



								#pragma endregion


									ImGui::EndTabItem();
								}


								ImGui::EndTabBar();
							}

							


							

							



						

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

#endif