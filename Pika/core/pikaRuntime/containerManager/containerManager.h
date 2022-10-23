#pragma once
#include <logs/log.h>
#include <runtimeContainer/runtimeContainer.h>

#include <unordered_map>
#include <string>

#include <dllLoader/dllLoader.h>

#include <windowSystemm/input.h>
#include <windowSystemm/window.h>

#include <pikaImgui/pikaImgui.h>
#include <pikaConsoleManager/pikaConsoleWindow.h>

namespace pika
{
	using containerId_t = unsigned int;
	
	struct ContainerManager
	{


		std::unordered_map<containerId_t, pika::RuntimeContainer> runningContainers;

		containerId_t createContainer(
			pika::ContainerInformation containerInformation,
			pika::LoadedDll &loadedDll, pika::LogManager &logManager, 
			pika::pikaImgui::ImGuiIdsManager &imguiIDsManager, size_t memoryPos = 0);

		containerId_t createContainer(
			std::string containerName,
			pika::LoadedDll &loadedDll, pika::LogManager &logManager,
			pika::pikaImgui::ImGuiIdsManager &imguiIDsManager, size_t memoryPos = 0);

		bool setSnapshotToContainer(pika::containerId_t containerId, const char* snapshotName,
			pika::LogManager &logManager, pika::pikaImgui::ImGuiIdsManager &imguiIdManager);

		bool setRecordingToContainer(pika::containerId_t containerId, const char *recordingName,
			pika::LogManager &logManager, pika::pikaImgui::ImGuiIdsManager &imguiIdManager);

		void* allocateContainerMemory(pika::RuntimeContainer &container, pika::ContainerInformation containerInformation, void *memPos = 0);

		//buffer should have the correct size
		void allocateContainerMemoryAtBuffer(pika::RuntimeContainer &container,
			pika::ContainerInformation containerInformation, void *buffer);


		//deallocates memory, does not call destructors
		void freeContainerStuff(pika::RuntimeContainer &container);

		void init();

		void update(
			pika::LoadedDll &loadedDll,
			pika::PikaWindow &window,
			pika::LogManager &logs,
			pika::pikaImgui::ImGuiIdsManager &imguiIdManager);

		void reloadDll(pika::LoadedDll &loadedDll,
			pika::PikaWindow &window,
			pika::LogManager &logs);

		bool destroyContainer(containerId_t id, pika::LoadedDll &loadedDll,
			pika::LogManager &logManager);

		bool makeSnapshot(containerId_t id, pika::LogManager &logManager, const char* fileName);

		bool startRecordingContainer(containerId_t id, pika::LogManager &logManager, const char *fileName);

		bool stopRecordingContainer(containerId_t id, pika::LogManager &logManager);

		bool makeRecordingStep(containerId_t id, pika::LogManager &logManager, 
			pika::Input &input);

		//same as destroy container but doesn't call user destructors
		bool forceTerminateContainer(containerId_t id, pika::LoadedDll &loadedDll,
			pika::LogManager &logManager);

		void destroyAllContainers(pika::LoadedDll &loadedDll,
			pika::LogManager &logManager);

		containerId_t idCounter = 0;

		//todo move outside
		void *allocateOSMemory(size_t size, void* baseAdress = 0);

		void deallocateOSMemory(void *baseAdress);
	};


	std::vector<std::string> getAvailableSnapshots(pika::RuntimeContainer &info);
	std::vector<std::string> getAvailableRecordings(pika::RuntimeContainer &info);

	std::vector<std::string> getAvailableSnapshotsAnyMemoryPosition(pika::RuntimeContainer &info);
	std::vector<std::string> getAvailableRecordingAnyMemoryPosition(pika::RuntimeContainer &info);

	std::vector<std::string> getAvailableSnapshotsAnyMemoryPosition(pika::ContainerInformation &info);
	std::vector<std::string> getAvailableRecordingsAnyMemoryPosition(pika::ContainerInformation &info);

	bool checkIfSnapshotIsCompatible(pika::RuntimeContainer &info, const char *snapshotName);

	bool checkIfSnapshotIsCompatibleAnyMemoryPosition(pika::RuntimeContainer &info, const char *snapshotName);

	bool checkIfSnapshotIsCompatibleAnyMemoryPosition(pika::ContainerInformation &info, const char *snapshotName);

	void *getSnapshotMemoryPosition(const char *snapshotName);

}