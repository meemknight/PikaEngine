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
	size_t totalSize = 0;

	//bonus allocators
	pika::StaticVector<pika::memory::FreeListAllocator, MaxAllocatorsCount> bonusAllocators = {};

	RequestedContainerInfo requestedContainerInfo = {};

	int imguiWindowId = 0;

	unsigned int frameCounter = 0;
	float frameTimer = 0;
	float currentMs = 0;

	struct FLAGS
	{
		enum
		{
			STATUS_PAUSE = 0,
			STATUS_RUNNING = 1,
			STATUS_BEING_RECORDED = 2,
			STATUS_BEING_PLAYBACK = 3,
		};

		int status = STATUS_RUNNING;

		bool shouldCallReaload = 0; //if the container happens to be on pause when the dll reloads we mark this to true

		char recordingName[256] = {};
		int frameNumber = 0;

		const char *getStatusName()
		{

			if (status == STATUS_RUNNING)
			{
				return "running.";
			}
			else if(status == STATUS_PAUSE)
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
			else if (status == STATUS_BEING_RECORDED)
			{
				return "on recording";
			}
			else if (status == STATUS_BEING_PLAYBACK)
			{
				return "on input playback";
			}
		}
		
		const char *getStatusIcon()
		{
			if (status == STATUS_RUNNING)
			{
				return ICON_FK_BOLT;
			}
			else if (status == STATUS_PAUSE)
			{
				return ICON_FK_PAUSE_CIRCLE_O;
			}
			else if (status == STATUS_BEING_RECORDED)
			{
				return ICON_FK_VIDEO_CAMERA;
			}
			else if (status == STATUS_BEING_PLAYBACK)
			{
				return ICON_FK_REPEAT;
			}
		}

	}flags;

};


}
