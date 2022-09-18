#include <containers.h>
#include <logs/assert.h>

//this should not allocate memory
Container *getContainer(const char *name, pika::memory::MemoryArena *memoryArena)
{

	if (std::strcmp(name, "Gameplay") == 0)
	{
		PIKA_PERMA_ASSERT(sizeof(Gameplay) == memoryArena->containerStructMemory.size, "invalid memory size for static data");

		return new(memoryArena->containerStructMemory.block)  Gameplay();
	}
	else
	{
		PIKA_PERMA_ASSERT(0, (std::string("invalid container name: ") + name).c_str());
		return nullptr;
	}

}