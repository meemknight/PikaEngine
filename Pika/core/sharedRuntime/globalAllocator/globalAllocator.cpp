#include "globalAllocator.h"
#include <malloc.h>
#include <pikaAllocator/freeListAllocator.h>
#include <fstream>
#include <logs/assert.h>


void *DefaultAllocator(size_t size)
{
	return malloc(size);
}
void DefaultFree(void *ptr)
{
	free(ptr);
}

void *DisabeledAllocator(size_t size)
{
	return 0;
}
void DisabeledFree(void *ptr)
{
	(void)ptr;
}

pika::memory::FreeListAllocator *currentCustomAllocator = {};
void *CustomAllocator(size_t size)
{
	return currentCustomAllocator->allocate(size);
}
void CustomFree(void *ptr)
{
	currentCustomAllocator->free(ptr);
}

void* (*GlobalAllocateFunction)(size_t) = DefaultAllocator;
void  (*GlobalFree)(void *) = DefaultFree;

namespace pika
{
namespace memory
{
	int pushed = 0;

	void setGlobalAllocatorToStandard()
	{
		PIKA_PERMA_ASSERT(!pushed, "Can't edit allocators while you pushed custom allocators to standard");

		GlobalAllocateFunction = DefaultAllocator;
		GlobalFree = DefaultFree;
	}

	void dissableAllocators()
	{
		PIKA_PERMA_ASSERT(!pushed, "Can't edit allocators while you pushed custom allocators to standard");

		GlobalAllocateFunction = DisabeledAllocator;
		GlobalFree = DisabeledFree;
	}

	void setGlobalAllocator(pika::memory::FreeListAllocator *allocator)
	{
		PIKA_PERMA_ASSERT(!pushed, "Can't edit allocators while you pushed custom allocators to standard");

		currentCustomAllocator = allocator;
		GlobalAllocateFunction = CustomAllocator;
		GlobalFree = CustomFree;
	}


	void pushCustomAllocatorsToStandard()
	{
		pushed++;
		GlobalAllocateFunction = DefaultAllocator;
		GlobalFree = DefaultFree;
	}

	//can be pushed and popped only once
	void popCustomAllocatorsToStandard()
	{
		pushed--;
		PIKA_PERMA_ASSERT(pushed>=0, "pop underflow on popCustomAllocatorsToStandard");
		GlobalAllocateFunction = CustomAllocator;
		GlobalFree = CustomFree;
	}
}
}



void *operator new  (size_t count)
{
	return GlobalAllocateFunction(count);
}

void *operator new[](size_t count)
{
	return GlobalAllocateFunction(count);
}

void operator delete  (void *ptr)
{

	GlobalFree(ptr);
}

void operator delete[](void *ptr)
{
	GlobalFree(ptr);
}