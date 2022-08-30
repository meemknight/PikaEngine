#pragma once


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


	};




};
};

