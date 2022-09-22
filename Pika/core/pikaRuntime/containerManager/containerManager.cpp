#include "containerManager.h"
#include "containerManager.h"
#include "containerManager.h"
#include <globalAllocator/globalAllocator.h>

bool pika::ContainerManager::createContainer
(std::string name, pika::ContainerInformation containerInformation,
	pika::DllLoader &dllLoader, pika::LogManager &logManager)
{

	if (runningContainers.find(name) != runningContainers.end())
	{
		logManager.log((std::string("Container name already exists: ") + name).c_str(), pika::LogManager::logError);
		return false;
	}


	pika::RuntimeContainer container = {};
	container.arena.allocateStaticMemory(containerInformation); //this just allocates the staic memory

	container.allocator.init(malloc(containerInformation.containerStaticInfo.defaultHeapMemorySize),
		containerInformation.containerStaticInfo.defaultHeapMemorySize);

	dllLoader.bindAllocatorDllRealm(&container.allocator);
	
	//this calls the constructors (from the dll realm)
	if (!dllLoader.constructRuntimeContainer(container, containerInformation.containerName.c_str()))
	{
		dllLoader.resetAllocatorDllRealm();

		logManager.log((std::string("Couldn't construct container: ") + name).c_str(), pika::LogManager::logError);

		container.arena.dealocateStaticMemory(); //static memory
		free(container.allocator.originalBaseMemory); //heap memory

		return false;
	}
	dllLoader.resetAllocatorDllRealm();

	
	dllLoader.bindAllocatorDllRealm(&container.allocator);
	container.pointer->create(); //this calls create() (from the dll realm)
	dllLoader.resetAllocatorDllRealm();//sets the global allocator back to standard (used for runtime realm)

	runningContainers[name] = container;

	return true;
}

void pika::ContainerManager::init()
{
}

void pika::ContainerManager::update(pika::DllLoader &dllLoader, pika::Input input, float deltaTime, pika::WindowState windowState)
{
	for (auto &c : runningContainers)
	{
		dllLoader.bindAllocatorDllRealm(&c.second.allocator);
		c.second.pointer->update(input, deltaTime, windowState);
		dllLoader.resetAllocatorDllRealm();

	}

}

bool pika::ContainerManager::destroyContainer(std::string name, pika::DllLoader &dllLoader,
	pika::LogManager &logManager)
{
	auto c = runningContainers.find(name);
	if (c == runningContainers.end())
	{
		logManager.log((std::string("Couldn't find container for destruction: ") + name).c_str(),
			pika::LogManager::logError);
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
