#pragma once
#include <windowSystemm/window.h>
#include <iostream>
#include <pikaOptional.h>
#include <string>
#include <pikaAllocator/freeListAllocator.h>
#include <staticVector.h>

//this is passed by the engine. You should not modify the data
struct RequestedContainerInfo
{
	pika::memory::FreeListAllocator *mainAllocator = {};


};

struct ContainerStaticInfo
{
	static constexpr size_t MaxAllocatorsCount = 128;

	//this is the main heap allocator memory size
	size_t defaultHeapMemorySize = 0;
	
	pika::StaticVector<size_t, MaxAllocatorsCount> bonusAllocators = {};

	bool _internalNotImplemented = 0;

	bool operator==(const ContainerStaticInfo &other)
	{
		if (this == &other) { return true; }

		return
			this->defaultHeapMemorySize == other.defaultHeapMemorySize &&
			this->bonusAllocators == other.bonusAllocators &&
			this->_internalNotImplemented == other._internalNotImplemented;
	}

	bool operator!=(const ContainerStaticInfo &other)
	{
		return !(*this == other);
	}


};


struct Container
{

	//this is used to give to the engine basic information about your container.
	//this function should be pure
	//this function should not allocate memory
	//this should not be dependent on anything that is called on create or library initialization
	static ContainerStaticInfo containerInfo() { ContainerStaticInfo c; c._internalNotImplemented = true; return c; };
	

	virtual void create(RequestedContainerInfo &requestedInfo) = 0;

	virtual void update(
		pika::Input input, 
		float deltaTime, 
		pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo) = 0;

	virtual ~Container() {};

};