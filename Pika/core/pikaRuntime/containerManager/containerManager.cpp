#include "containerManager.h"
#include "containerManager.h"
#include "containerManager.h"
#include <globalAllocator/globalAllocator.h>

bool pika::ContainerManager::createContainer
(std::string name, pika::ContainerInformation containerInformation, pika::DllLoader &dllLoader)
{

	if (runningContainers.find(name) != runningContainers.end())
	{
		//todo log error
		return false;
	}


	pika::RuntimeContainer container;
	container.arena.allocateStaticMemory(containerInformation); //this just allocates the memory

	container.allocator.init(malloc(10000), 10000); //todo 

	dllLoader.bindAllocatorDllRealm(&container.allocator);
	 //this calls the constructors (from the dll realm)
	if (!dllLoader.constructRuntimeContainer(container, containerInformation.containerName.c_str()))
	{
		dllLoader.resetAllocatorDllRealm();


		//todo log error

		container.arena.dealocateStaticMemory();

		free(container.allocator.originalBaseMemory);

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

bool pika::ContainerManager::destroyContainer(std::string name, pika::DllLoader &dllLoader)
{
	auto c = runningContainers.find(name);
	if (c == runningContainers.end())
	{
		//todo log error
		return false;
	}

	dllLoader.bindAllocatorDllRealm(&c->second.allocator);
	dllLoader.destructContainer_(&(c->second.pointer), &c->second.arena);
	dllLoader.resetAllocatorDllRealm();

	c->second.arena.dealocateStaticMemory();

	free(c->second.allocator.originalBaseMemory);

	return true;
}

void pika::ContainerManager::destroyAllContainers(pika::DllLoader &dllLoader)
{
	for (auto &c : runningContainers)
	{
		destroyContainer(c.first, dllLoader);
	}
}
