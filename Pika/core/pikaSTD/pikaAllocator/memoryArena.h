#pragma once
#include <containerInformation.h>

namespace pika
{
namespace memory
{


	struct MemoryBlock
	{
		size_t size = 0;
		void *block = 0;
	};


	struct MemoryArena
	{

		//this is used to allocate the static memory of the container (struct member data)
		MemoryBlock containerStructMemory = {};

		//todo this will request an allocator probably in the future
		void allocateStaticMemory(const ContainerInformation &containerInfo);
		void dealocateStaticMemory();
	};




};
};

