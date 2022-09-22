#include <containers.h>
#include <logs/assert.h>

//this should not allocate memory
Container *getContainer(const char *name, pika::memory::MemoryArena *memoryArena)
{

	if (std::strcmp(name, "Gameplay") == 0)
	{
		if (sizeof(Gameplay) != memoryArena->containerStructMemory.size)
		{
			//"invalid memory size for static data"
			return nullptr;
		}

		return new(memoryArena->containerStructMemory.block)  Gameplay();
	}
	else
	{
		//"invalid container name: "
		return nullptr;
	}

}