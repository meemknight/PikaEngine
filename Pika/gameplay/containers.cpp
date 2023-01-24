#include <containers.h>
#include <logs/assert.h>


#define PIKA_DECLARE_CONTAINER(x)	if (std::strcmp(name, #x ) == 0)						\
{																							\
if (sizeof(x) != memoryArena->containerStructMemory.size) { return nullptr; }				\
	return new(memoryArena->containerStructMemory.block)  x ();								\
}																							\
else


//this should not allocate memory
Container *getContainer(const char *name, pika::memory::MemoryArena *memoryArena)
{

	PIKA_ALL_CONTAINERS()
	{
		//"invalid container name: "
		return nullptr;
	}

}

#undef PIKA_DECLARE_CONTAINER