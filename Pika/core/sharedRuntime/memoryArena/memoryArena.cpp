#include <memoryArena/memoryArena.h>
#include <malloc.h>

//todo remove
void pika::memory::MemoryArena::allocateStaticMemory(const ContainerInformation &containerInfo)
{
	containerStructMemory.size = containerInfo.containerStructBaseSize;
	containerStructMemory.block = malloc(containerInfo.containerStructBaseSize);
}

void pika::memory::MemoryArena::dealocateStaticMemory()
{
	containerStructMemory.size = 0;
	free(containerStructMemory.block);
	containerStructMemory.block = 0;
}

