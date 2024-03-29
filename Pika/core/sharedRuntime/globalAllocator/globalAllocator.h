#pragma once

#include <pikaAllocator/freeListAllocator.h>

namespace pika
{
namespace memory
{
	void setGlobalAllocatorToStandard();
	
	void setGlobalAllocator(pika::memory::FreeListAllocator *allocator);

	void dissableAllocators();

	void pushCustomAllocatorsToStandard();

	void popCustomAllocatorsToStandard();

}
}
