#pragma once
#include <pikaConfig.h>
#include <string>
#include <baseContainer.h>
#include <memoryArena/memoryArena.h>
#include <pikaAllocator/freeListAllocator.h>
#include <IconsForkAwesome.h>

namespace pika
{


struct RuntimeContainer
{
	//this is the base adress of the runtime container. here is the beginning of all the allocated memory
	void *getBaseAdress() { return arena.containerStructMemory.block; };

	char baseContainerName[50] = {};
	//std::string name = {};

	//this is the pointer to the container virtual class
	Container *pointer = {};

	//this is the container memory arena. here we have all the static data of the container
	pika::memory::MemoryArena arena = {};

	//this is the allocator of the arena.
	pika::memory::FreeListAllocator allocator = {};
	size_t allocatorSize = 0;
	size_t totalSize = 0;

	//bonus allocators
	pika::StaticVector<pika::memory::FreeListAllocator, MaxAllocatorsCount> bonusAllocators = {};


	RequestedContainerInfo requestedContainerInfo = {};

	struct
	{
		bool running = 1;
		bool shouldCallReaload = 0; //if the container happens to be on pause when the dll reloads we mark this to true

		const char *getStatusName()
		{

			if (running)
			{
				return "running.";
			}
			else
			{
				if (shouldCallReaload)
				{
					return "paused, waiting reload.";
				}
				else
				{
					return "paused.";
				}

			}
		}
		
		const char *getStatusIcon()
		{
			if (running)
			{
				return ICON_FK_BOLT;
			}
			else
			{
				return ICON_FK_PAUSE_CIRCLE_O;
			}
		}

	}flags;

};


}
