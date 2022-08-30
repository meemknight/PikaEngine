#pragma once
#include <pikaConfig.h>
#include <string>
#include <baseContainer.h>
#include <pikaAllocator/memoryArena.h>

struct RuntimeContainer
{
	std::string baseContainerName = {};
	//std::string name = {};
	Container *pointer = {};

	pika::memory::MemoryArena arena = {};
};