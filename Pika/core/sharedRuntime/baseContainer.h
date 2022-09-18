#pragma once
#include <windowSystemm/window.h>

struct Container
{

	virtual void create() = 0;

	virtual void update(
		pika::Input input, 
		float deltaTime, 
		pika::WindowState windowState) = 0;

	virtual ~Container() {};

};