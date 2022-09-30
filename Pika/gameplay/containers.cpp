#include <containers.h>
#include <logs/assert.h>


#define PIKA_MAKE_CONTAINER_GET(x)	if (std::strcmp(name, #x ) == 0)						\
{																							\
if (sizeof(x) != memoryArena->containerStructMemory.size) { return nullptr; }				\
	return new(memoryArena->containerStructMemory.block)  x ();								\
}																							\
else


//this should not allocate memory
Container *getContainer(const char *name, pika::memory::MemoryArena *memoryArena)
{

	PIKA_MAKE_CONTAINER_GET(Gameplay)
	PIKA_MAKE_CONTAINER_GET(ImmageViewer)
	{
		//"invalid container name: "
		return nullptr;
	}

}