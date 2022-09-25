#include "containerManager.h"
#include "containerManager.h"
#include "containerManager.h"
#include <globalAllocator/globalAllocator.h>

pika::containerId_t pika::ContainerManager::createContainer
(pika::ContainerInformation containerInformation,
	pika::DllLoader &dllLoader, pika::LogManager &logManager)
{	
	containerId_t id = ++idCounter;
	
	//not necessary if this is the only things that assigns ids.
	//if (runningContainers.find(id) != runningContainers.end())
	//{
	//	logManager.log((std::string("Container id already exists: #") + std::to_string(id)).c_str(), pika::logError);
	//	return false;
	//}

	pika::RuntimeContainer container = {};
	container.arena.allocateStaticMemory(containerInformation); //this just allocates the staic memory

	container.allocator.init(malloc(containerInformation.containerStaticInfo.defaultHeapMemorySize),
		containerInformation.containerStaticInfo.defaultHeapMemorySize);

	dllLoader.bindAllocatorDllRealm(&container.allocator);
	
	//this calls the constructors (from the dll realm)
	if (!dllLoader.constructRuntimeContainer(container, containerInformation.containerName.c_str()))
	{
		dllLoader.resetAllocatorDllRealm();

		logManager.log((std::string("Couldn't construct container: #") + std::to_string(id)).c_str(), pika::logError);

		container.arena.dealocateStaticMemory(); //static memory
		free(container.allocator.originalBaseMemory); //heap memory

		return 0;
	}
	dllLoader.resetAllocatorDllRealm();


#pragma region setup requested container info

	container.requestedContainerInfo.mainAllocator = &container.allocator;

#pragma endregion

	
	dllLoader.bindAllocatorDllRealm(&container.allocator);
	container.pointer->create(container.requestedContainerInfo); //this calls create() (from the dll realm)
	dllLoader.resetAllocatorDllRealm();//sets the global allocator back to standard (used for runtime realm)

	runningContainers[id] = container;

	return id;
}

void pika::ContainerManager::init()
{
}

void pika::ContainerManager::update(pika::DllLoader &dllLoader,
	pika::Input input, float deltaTime, pika::WindowState windowState)
{
	for (auto &c : runningContainers)
	{
		dllLoader.bindAllocatorDllRealm(&c.second.allocator);
		c.second.pointer->update(input, deltaTime, windowState, c.second.requestedContainerInfo);
		dllLoader.resetAllocatorDllRealm();

	}

}

bool pika::ContainerManager::destroyContainer(containerId_t id, pika::DllLoader &dllLoader,
	pika::LogManager &logManager)
{
	auto c = runningContainers.find(id);
	if (c == runningContainers.end())
	{
		logManager.log((std::string("Couldn't find container for destruction: #") + std::to_string(id)).c_str(),
			pika::logError);
		return false;
	}

	dllLoader.bindAllocatorDllRealm(&c->second.allocator);
	dllLoader.destructContainer_(&(c->second.pointer), &c->second.arena);
	dllLoader.resetAllocatorDllRealm();

	c->second.arena.dealocateStaticMemory(); //static memory
	free(c->second.allocator.originalBaseMemory); //heap memory

	runningContainers.erase(c);

	return true;
}

void pika::ContainerManager::destroyAllContainers(pika::DllLoader &dllLoader,
	pika::LogManager &logManager)
{
	for (auto &c : runningContainers)
	{
		destroyContainer(c.first, dllLoader, logManager);
	}
}
