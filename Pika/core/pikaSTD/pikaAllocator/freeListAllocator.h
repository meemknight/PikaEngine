//////////////////////////////////////////////////
//freeListAllocator.h				beta 0.3
//Copyright(c) 2020 Luta Vlad
//https://github.com/meemknight/freeListAllocator
//////////////////////////////////////////////////

#pragma once

#include <pikaConfig.h>
#include <pikaSizes.h>

namespace pika
{

namespace memory
{


	#ifdef PIKA_WINDOWS
	
		struct FreeListAllocatorMutex
		{
		
			FreeListAllocatorMutex();
		
			void lock();
			void unlock();
		
			long counter;
			void* semaphore;
		
			~FreeListAllocatorMutex();
			
			FreeListAllocatorMutex(FreeListAllocatorMutex &other) = delete;
			FreeListAllocatorMutex(FreeListAllocatorMutex &&other) = delete;
			FreeListAllocatorMutex &operator=(const FreeListAllocatorMutex &other) = delete;

		};
	
	#elif defined(PIKA_LINUX)
	
	//todo add linux support
	
	#endif
	
	
	struct FreeListAllocator
	{
		char* baseMemory = 0;
		void* originalBaseMemory = 0;
		size_t heapSize = 0;

		FreeListAllocator() = default;
		FreeListAllocator(void* baseMemory, size_t memorySize)
		{
			init(baseMemory, memorySize);
		}
	
		void init(void* baseMemory, size_t memorySize);
	
		void* allocate(size_t size);
	
		void free(void* mem);
	
		//void* threadSafeAllocate(size_t size);
		//void threadSafeFree(void* mem);
	
		//available memory is the free memory
		//biggest block is how large is the biggest free memory block
		//you can allocate less than the largest biggest free memory because 16 bytes are reserved per block
		void calculateMemoryMetrics(size_t& availableMemory, size_t& biggestBlock, int& freeBlocks);
	
	
		//if this is false it will crash if it is out of memory
		//if this is true it will return 0 when there is no more memory
		//I rocommand leaving this to false
		bool returnZeroIfNoMoreMemory = false;
	

		bool operator==(const FreeListAllocator &other)
		{
			if (this == &other) { return true; }

			char *baseMemory = 0;
			void *originalBaseMemory = 0;
			size_t heapSize = 0;
			void *end = 0;

			if(
				baseMemory == other.baseMemory &&
				originalBaseMemory == other.originalBaseMemory &&
				heapSize == other.heapSize &&
				end == other.end
				)
			{
				return true;
			}

			return false;
		}

		bool operator!=(const FreeListAllocator &other)
		{
			return !(*this == other);
		}

	private:
	
		void* end = 0;
	
		//FreeListAllocatorMutex mu;
	
		size_t getEnd()
		{
			return (size_t)end;
		}
	
	};


};

};