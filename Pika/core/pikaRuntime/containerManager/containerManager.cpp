#include "containerManager.h"
#include "containerManager.h"
#include "containerManager.h"
#include <globalAllocator/globalAllocator.h>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <fileManipulation/fileManipulation.h>
#include <stringManipulation/stringManipulation.h>
#include <filesystem>

pika::containerId_t pika::ContainerManager::createContainer(std::string containerName, 
	pika::LoadedDll &loadedDll, pika::LogManager &logManager)
{
	
	for(auto &i : loadedDll.containerInfo)
	{
		
		if (i.containerName == containerName)
		{
			return createContainer(i, loadedDll, logManager);
		}

	}
	
	logManager.log(("Couldn't create container, couldn't find the name: " + containerName).c_str(), pika::logError);

	return 0;
}

bool pika::ContainerManager::setSnapshotToContainer(pika::containerId_t containerId, const char *snapshotName, pika::LogManager &logManager)
{
	auto c = runningContainers.find(containerId);
	if (c == runningContainers.end())
	{
		logManager.log((std::string("Couldn't find container for setting snapshot: #") 
			+ std::to_string(containerId)).c_str(),
			pika::logError);
		return false;
	}

	if (!checkIfSnapshotIsCompatible(c->second, snapshotName))
	{
		logManager.log((std::string("Snapshot incompatible: ") + snapshotName).c_str(),
			pika::logError);
		return false;
	}
	
	std::string file = PIKA_ENGINE_RESOURCES_PATH;
	file += snapshotName;
	file += ".snapshot";

	auto s = pika::getFileSize(file.c_str());

	if (s != (c->second.totalSize + sizeof(pika::RuntimeContainer)) ) 
	{
		logManager.log((std::string("Snapshot corrupted probably, file size incorrect: ") 
			+ snapshotName).c_str(),
			pika::logError);
		return false;
	}

	pika::readEntireFile(file.c_str(), c->second.getBaseAdress(),
		c->second.totalSize, sizeof(pika::RuntimeContainer));

	logManager.log("Loaded snapshot");
	return true;
}

//todo use regions further appart in production
void* pika::ContainerManager::allocateContainerMemory(pika::RuntimeContainer &container,
	pika::ContainerInformation containerInformation, void *memPos)
{
	size_t memoryRequired = containerInformation.calculateMemoryRequirements();

	void * baseMemory = allocateOSMemory(memoryRequired, memPos);

	if (baseMemory == nullptr) { return 0; }

	container.totalSize = memoryRequired;

	allocateContainerMemoryAtBuffer(container,
		containerInformation, baseMemory);

	return baseMemory;
}

void pika::ContainerManager::allocateContainerMemoryAtBuffer(pika::RuntimeContainer &container,
	pika::ContainerInformation containerInformation, void *buffer)
{
	const size_t staticMemory = containerInformation.containerStructBaseSize;
	const size_t heapMemory = containerInformation.containerStaticInfo.defaultHeapMemorySize;

	char *currentMemoryAdress = (char *)buffer;

	container.arena.containerStructMemory.size = staticMemory;
	container.arena.containerStructMemory.block = currentMemoryAdress;
	currentMemoryAdress += staticMemory;
	pika::align64(currentMemoryAdress);

	container.allocator.init(currentMemoryAdress, heapMemory);

	currentMemoryAdress += heapMemory;

	for (int i = 0; i < containerInformation.containerStaticInfo.bonusAllocators.size(); i++)
	{
		pika::align64(currentMemoryAdress);

		pika::memory::FreeListAllocator allocator;
		allocator.init(
			currentMemoryAdress,
			containerInformation.containerStaticInfo.bonusAllocators[i]
		);
		container.bonusAllocators.push_back(allocator);
		currentMemoryAdress += containerInformation.containerStaticInfo.bonusAllocators[i];
	}

}

void pika::ContainerManager::freeContainerStuff(pika::RuntimeContainer &container)
{
	deallocateOSMemory(container.arena.containerStructMemory.block);

	//container.arena.dealocateStaticMemory(); //static memory
	//deallocateOSMemory(container.allocator.originalBaseMemory); //heap memory
	//
	//for (auto &i : container.bonusAllocators)
	//{
	//	deallocateOSMemory(i.originalBaseMemory);
	//}
}

pika::containerId_t pika::ContainerManager::createContainer
(pika::ContainerInformation containerInformation,
	pika::LoadedDll &loadedDll, pika::LogManager &logManager)
{	
	containerId_t id = ++idCounter;
	
	//not necessary if this is the only things that assigns ids.
	//if (runningContainers.find(id) != runningContainers.end())
	//{
	//	logManager.log((std::string("Container id already exists: #") + std::to_string(id)).c_str(), pika::logError);
	//	return false;
	//}

	//todo a create and destruct wrapper

	pika::RuntimeContainer container = {};
	pika::strlcpy(container.baseContainerName, containerInformation.containerName,
		sizeof(container.baseContainerName));
	
	if (!allocateContainerMemory(container, containerInformation))
	{
		logManager.log((std::string("Couldn't allocate memory for constructing container: #") 
			+ std::to_string(id)).c_str(), pika::logError);
		return 0;
	}

	loadedDll.bindAllocatorDllRealm(&container.allocator);
	
	//this calls the constructors (from the dll realm)
	if (!loadedDll.constructRuntimeContainer(container, containerInformation.containerName.c_str()))
	{
		loadedDll.resetAllocatorDllRealm();

		logManager.log((std::string("Couldn't construct container: #") + std::to_string(id)).c_str(), pika::logError);

		freeContainerStuff(container);

		return 0;
	}


	loadedDll.resetAllocatorDllRealm();


#pragma region setup requested container info

	container.requestedContainerInfo.mainAllocator = &container.allocator;
	container.requestedContainerInfo.bonusAllocators = &container.bonusAllocators;

#pragma endregion

	
	loadedDll.bindAllocatorDllRealm(&container.allocator);
	container.pointer->create(container.requestedContainerInfo); //this calls create() (from the dll realm)
	loadedDll.resetAllocatorDllRealm();//sets the global allocator back to standard (used for runtime realm)

	runningContainers[id] = container;

	logManager.log(("Created container: " + std::string(container.baseContainerName) ).c_str());

	return id;
}

void pika::ContainerManager::init()
{
}

void pika::ContainerManager::update(pika::LoadedDll &loadedDll, pika::PikaWindow &window, pika::LogManager &logs)
{
	PIKA_DEVELOPMENT_ONLY_ASSERT(loadedDll.dllHand != 0, "dll not loaded when trying to update containers");

#pragma region reload dll


	//todo try to recover from a failed load

	if (loadedDll.shouldReloadDll())
	{
		reloadDll(loadedDll, window, logs); //todo return 0 on fail

		//todo
		//for (auto &c : runningContainers)
		//{
		//	if (c.second.flags.running)
		//	{
		//		loadedDll.bindAllocatorDllRealm(&c.second.allocator);
		//		//c.second.pointer->reload...
		//		loadedDll.resetAllocatorDllRealm();
		//	}
		//	else
		//	{
		//		c.second.flags.shouldCallReaload = true;
		//	}
		//
		//	
		//}
	}

	//if (loadedDll.shouldReloadDll() || window.input.buttons[pika::Button::P].released())
	//{
	//	pika::LoadedDll newDll = {};
	//
	//
	//	newDll.tryToloadDllUntillPossible(!loadedDll.id, logs); //create a new copy with a new id 
	//
	//#if 0
	//	//clear containers that dissapeared
	//	{
	//		std::unordered_set<std::string> containerNames;
	//		for (auto &c : newDll.containerInfo)
	//		{
	//			containerNames.insert(c.containerName);
	//		}
	//
	//		std::vector<pika::containerId_t> containersToClean;
	//		for (auto &i : runningContainers)
	//		{
	//			if (containerNames.find(i.second.baseContainerName) ==
	//				containerNames.end())
	//			{
	//				std::string l = "Killed container because it does not exist anymore in dll: " + i.second.baseContainerName
	//					+ " #" + std::to_string(i.first);
	//				logs.log(l.c_str(), pika::logError);
	//
	//				containersToClean.push_back(i.first);
	//			}
	//		}
	//
	//		for (auto i : containersToClean)
	//		{
	//			destroyContainer(i, loadedDll, logs);
	//		}
	//	}
	//#endif
	//
	//	//set new dll
	//	loadedDll.unloadDll();
	//
	//	loadedDll = newDll;
	//		
	//	loadedDll.gameplayReload_(window.context);
	//}

	
#pragma endregion


#pragma region running containers
	for (auto &c : runningContainers)
	{

		if (c.second.flags.running)
		{
			loadedDll.bindAllocatorDllRealm(&c.second.allocator);
			c.second.pointer->update(window.input, window.deltaTime, window.windowState, c.second.requestedContainerInfo);
			loadedDll.resetAllocatorDllRealm();
		}

	}
#pragma endregion

}

void pika::ContainerManager::reloadDll(pika::LoadedDll &loadedDll, pika::PikaWindow &window, pika::LogManager &logs)
{

	std::this_thread::sleep_for(std::chrono::milliseconds(200)); // make sure that the compiler had enough time 
		//to get hold onto the dll 


	//pika::LoadedDll newDll;
	//if (!newDll.tryToloadDllUntillPossible(loadedDll.id + 1, logs, std::chrono::seconds(1)))
	//{
	//	logs.log("Dll reload attemp failed", pika::logWarning);
	//	newDll.unloadDll();
	//	return;
	//}
	//else
	//{
	//	newDll.unloadDll();
	//}
	//std::this_thread::sleep_for(std::chrono::milliseconds(10)); // make sure that the dll is unloaded


	auto oldContainerInfo = loadedDll.containerInfo;

	if (!loadedDll.tryToloadDllUntillPossible(loadedDll.id, logs, std::chrono::seconds(5)))
	{
		logs.log("Couldn't reloaded dll", pika::logWarning);
		return;
	}
	//todo pospone dll reloading

	std::unordered_map<std::string, pika::ContainerInformation> containerNames;
	for (auto &c : loadedDll.containerInfo)
	{
		containerNames[c.containerName] = c;
	}

	std::unordered_map<std::string, pika::ContainerInformation> oldContainerNames;
	for (auto &c : oldContainerInfo)
	{
		oldContainerNames[c.containerName] = c;
	}

	//clear containers that dissapeared
	{


		std::vector<pika::containerId_t> containersToClean;
		for (auto &i : runningContainers)
		{
			if (containerNames.find(i.second.baseContainerName) ==
				containerNames.end())
			{
				std::string l = "Killed container because it does not exist anymore in dll: " + 
					std::string(i.second.baseContainerName)
					+ " #" + std::to_string(i.first);
				logs.log(l.c_str(), pika::logError);

				containersToClean.push_back(i.first);
			}
		}

		for (auto i : containersToClean)
		{
			forceTerminateContainer(i, loadedDll, logs);
		}
	}

	//clear containers that changed static info
	{

		std::vector<pika::containerId_t> containersToClean;
		for (auto &i : runningContainers)
		{

			auto &newContainer = containerNames[i.second.baseContainerName];
			auto &oldContainer = oldContainerNames[i.second.baseContainerName];

			if (newContainer != oldContainer)
			{
				std::string l = "Killed container because its static container info\nhas changed: "
					+ std::string(i.second.baseContainerName)
					+ " #" + std::to_string(i.first);
				logs.log(l.c_str(), pika::logError);

				containersToClean.push_back(i.first);
			}

		}

		for (auto i : containersToClean)
		{
			forceTerminateContainer(i, loadedDll, logs);
		}

	}


	loadedDll.gameplayReload_(window.context);

	logs.log("Reloaded dll");

}

bool pika::ContainerManager::destroyContainer(containerId_t id, pika::LoadedDll &loadedDll,
	pika::LogManager &logManager)
{
	PIKA_DEVELOPMENT_ONLY_ASSERT(loadedDll.dllHand != 0, "dll not loaded when trying to destroy container");

	auto c = runningContainers.find(id);
	if (c == runningContainers.end())
	{
		logManager.log((std::string("Couldn't find container for destruction: #") + std::to_string(id)).c_str(),
			pika::logError);
		return false;
	}

	auto name = c->second.baseContainerName;

	loadedDll.bindAllocatorDllRealm(&c->second.allocator);
	loadedDll.destructContainer_(&(c->second.pointer), &c->second.arena);
	loadedDll.resetAllocatorDllRealm();

	freeContainerStuff(c->second);

	runningContainers.erase(c);

	logManager.log((std::string("Destroyed continer: ") + name + " #" + std::to_string(id)).c_str());

	return true;
}

//todo remove some of this functions in production


//snapshot file format:
//
// binary
// 
// pika::RuntimeContainer 
// 
// static memory
// 
// heap memory
// 
//
bool pika::ContainerManager::makeSnapshot(containerId_t id, pika::LogManager &logManager, const char *fileName)
{
	auto c = runningContainers.find(id);
	if (c == runningContainers.end())
	{
		logManager.log((std::string("Couldn't find container for making snapshot: #") + std::to_string(id)).c_str(),
			pika::logError);
		return false;
	}


	std::string filePath = PIKA_ENGINE_RESOURCES_PATH;
	filePath += fileName;

	filePath += ".snapshot";
		

	if(!pika::writeEntireFile(filePath.c_str(), &c->second, sizeof(c->second)))
	{
		logManager.log(("Couldn't write to file for making snapshot: " + filePath).c_str(),
			pika::logError);
		return false;
	}

	if (!pika::appendToFile(filePath.c_str(), 
		c->second.getBaseAdress(), c->second.totalSize))
	{
		pika::deleteFile(filePath.c_str());
		logManager.log(("Couldn't write to file for making snapshot: " + filePath).c_str(),
			pika::logError);
		return false;
	}


	//if (!pika::appendToFile(filePath.c_str(), 
	//	c->second.arena.containerStructMemory.block, c->second.arena.containerStructMemory.size))
	//{
	//	pika::deleteFile(filePath.c_str());
	//	logManager.log(("Couldn't write to file for making snapshot: " + filePath).c_str(),
	//		pika::logError);
	//	return false;
	//}
	//
	//if (!pika::appendToFile(filePath.c_str(),
	//	c->second.allocator.originalBaseMemory, c->second.allocatorSize))
	//{
	//	pika::deleteFile(filePath.c_str());
	//	logManager.log(("Couldn't write to file for making snapshot: " + filePath).c_str(),
	//		pika::logError);
	//	return false;
	//}


	return true;
}

bool pika::ContainerManager::forceTerminateContainer(containerId_t id, pika::LoadedDll &loadedDll, pika::LogManager &logManager)
{
	PIKA_DEVELOPMENT_ONLY_ASSERT(loadedDll.dllHand != 0, "dll not loaded when trying to destroy container");

	auto c = runningContainers.find(id);
	if (c == runningContainers.end())
	{
		logManager.log((std::string("Couldn't find container for destruction: #") + std::to_string(id)).c_str(),
			pika::logError);
		return false;
	}

	auto name = c->second.baseContainerName;

	freeContainerStuff(c->second);

	runningContainers.erase(c);

	logManager.log((std::string("Force terminated continer: ") + name + " #" + std::to_string(id)).c_str());

	return true;
}

void pika::ContainerManager::destroyAllContainers(pika::LoadedDll &loadedDll,
	pika::LogManager &logManager)
{
	std::vector < pika::containerId_t> containersId;
	containersId.reserve(runningContainers.size());

	for (auto &c : runningContainers)
	{
		containersId.push_back(c.first);
	}

	for (auto i : containersId)
	{
		destroyContainer(i, loadedDll, logManager);
	}

}

#ifdef PIKA_PRODUCTION

void *pika::ContainerManager::allocateOSMemory(size_t size, void *baseAdress)
{
	PIKA_PERMA_ASSERT(baseAdress == nullptr, "can't allocate fixed memory in production");
	return malloc(size);
}

void pika::ContainerManager::deallocateOSMemory(void *baseAdress)
{
	free(baseAdress);
}

#else

#include <Windows.h>

void *pika::ContainerManager::allocateOSMemory(size_t size, void *baseAdress)
{
	return VirtualAlloc(baseAdress, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void pika::ContainerManager::deallocateOSMemory(void *baseAdress)
{
	VirtualFree(baseAdress, 0, MEM_RELEASE);
}



#endif

std::vector<std::string> pika::getAvailableSnapshots(pika::RuntimeContainer &info)
{
	std::vector<std::string> files;

	auto curDir = std::filesystem::directory_iterator(PIKA_ENGINE_RESOURCES_PATH);

	for (const auto &iter : curDir)
	{
		if (std::filesystem::is_regular_file(iter)
			&& iter.path().extension() == ".snapshot"
			)
		{
			if (pika::checkIfSnapshotIsCompatible(info, iter.path().stem().string().c_str()))
			{
				files.push_back(iter.path().stem().string());
			}

		}
	}

	return files;
}

bool pika::checkIfSnapshotIsCompatible(pika::RuntimeContainer &info, const char *snapshotName)
{

	std::string file = PIKA_ENGINE_RESOURCES_PATH;
	file += snapshotName;
	file += ".snapshot";
	
	pika::RuntimeContainer loadedInfo = {};

	auto s = pika::readEntireFile(file.c_str(), &loadedInfo, sizeof(loadedInfo));

	if (s != sizeof(loadedInfo))
	{
		return 0;
	}

	//todo mabe a method here?
	if (loadedInfo.allocator.originalBaseMemory != info.allocator.originalBaseMemory)
	{
		return false;
	}

	if (loadedInfo.arena.containerStructMemory.block != info.arena.containerStructMemory.block)
	{
		return false;
	}

	if (loadedInfo.arena.containerStructMemory.size != info.arena.containerStructMemory.size)
	{
		return false;
	}

	if (std::strcmp(loadedInfo.baseContainerName, info.baseContainerName) != 0)
	{
		return false;
	}

	if (loadedInfo.requestedContainerInfo.bonusAllocators != info.requestedContainerInfo.bonusAllocators)
	{
		return false;
	}

	if (loadedInfo.totalSize != info.totalSize)
	{
		return false;
	}
		
	return true;
}
