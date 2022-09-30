#pragma once
#include <pikaConfig.h>
#include <string>
#include <baseContainer.h>
#include <memoryArena/memoryArena.h>
#include <pikaAllocator/freeListAllocator.h>

namespace pika
{


struct RuntimeContainer
{
	std::string baseContainerName = {};
	//std::string name = {};

	//this is the pointer to the container virtual class
	Container *pointer = {};

	//this is the container memory arena. here we have all the static data of the container
	pika::memory::MemoryArena arena = {};

	//this is the allocator of the arena.
	pika::memory::FreeListAllocator allocator = {};

	RequestedContainerInfo requestedContainerInfo = {};

};


}
