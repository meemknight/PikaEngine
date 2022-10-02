#include "containerManager.h"
#include "containerManager.h"
#include "containerManager.h"
#include <globalAllocator/globalAllocator.h>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <unordered_map>

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

	pika::RuntimeContainer container = {};
	container.baseContainerName = containerInformation.containerName;
	
	container.arena.allocateStaticMemory(containerInformation); //this just allocates the staic memory

	container.allocator.init(malloc(containerInformation.containerStaticInfo.defaultHeapMemorySize),
		containerInformation.containerStaticInfo.defaultHeapMemorySize);

	loadedDll.bindAllocatorDllRealm(&container.allocator);
	
	//this calls the constructors (from the dll realm)
	if (!loadedDll.constructRuntimeContainer(container, containerInformation.containerName.c_str()))
	{
		loadedDll.resetAllocatorDllRealm();

		logManager.log((std::string("Couldn't construct container: #") + std::to_string(id)).c_str(), pika::logError);

		container.arena.dealocateStaticMemory(); //static memory
		free(container.allocator.originalBaseMemory); //heap memory

		return 0;
	}
	loadedDll.resetAllocatorDllRealm();


#pragma region setup requested container info

	container.requestedContainerInfo.mainAllocator = &container.allocator;

#pragma endregion

	
	loadedDll.bindAllocatorDllRealm(&container.allocator);
	container.pointer->create(container.requestedContainerInfo); //this calls create() (from the dll realm)
	loadedDll.resetAllocatorDllRealm();//sets the global allocator back to standard (used for runtime realm)

	runningContainers[id] = container;

	logManager.log(("Created container: " + container.baseContainerName).c_str());

	return id;
}

void pika::ContainerManager::init()
{
}

void pika::ContainerManager::update(pika::LoadedDll &loadedDll, pika::PikaWindow &window, pika::LogManager &logs)
{
	PIKA_DEVELOPMENT_ONLY_ASSERT(loadedDll.dllHand != 0, "dll not loaded when trying to update containers");

#pragma region reload dll

	//todo button to reload dll

	//todo try to recover from a failed load

	if (loadedDll.shouldReloadDll())
	{
		reloadDll(loadedDll, window, logs);
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
		loadedDll.bindAllocatorDllRealm(&c.second.allocator);
		c.second.pointer->update(window.input, window.deltaTime, window.windowState, c.second.requestedContainerInfo);
		loadedDll.resetAllocatorDllRealm();

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
				std::string l = "Killed container because it does not exist anymore in dll: " + i.second.baseContainerName
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
					+ i.second.baseContainerName
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

	c->second.arena.dealocateStaticMemory(); //static memory
	free(c->second.allocator.originalBaseMemory); //heap memory

	runningContainers.erase(c);

	logManager.log((std::string("Destroyed continer: ") + name + " #" + std::to_string(id)).c_str());

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

	c->second.arena.dealocateStaticMemory(); //static memory
	free(c->second.allocator.originalBaseMemory); //heap memory

	runningContainers.erase(c);

	logManager.log((std::string("Force terminated continer: ") + name + " #" + std::to_string(id)).c_str());

	return true;
}

void pika::ContainerManager::destroyAllContainers(pika::LoadedDll &loadedDll,
	pika::LogManager &logManager)
{
	for (auto &c : runningContainers)
	{
		destroyContainer(c.first, loadedDll, logManager);
	}
}
