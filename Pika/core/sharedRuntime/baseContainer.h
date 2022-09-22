#pragma once
#include <windowSystemm/window.h>
#include <iostream>
#include <pikaOptional.h>
#include <string>

struct ContainerStaticInfo
{

	//this is the main allocator memory size
	size_t defaultHeapMemorySize = 0;


};


struct Container
{

	//this is used to give to the engine basic information about your container.
	//this function should be pure
	//this function should not allocate memory
	static ContainerStaticInfo containerInfo() {}; //todo not implemented check

	virtual void create() = 0;

	virtual void update(
		pika::Input input, 
		float deltaTime, 
		pika::WindowState windowState) = 0;

	virtual ~Container() {};

};