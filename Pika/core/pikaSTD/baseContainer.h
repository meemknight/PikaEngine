#pragma once
#include <windowSystemm/window.h>


struct Container
{

	virtual void create() = 0;

	virtual void update(pika::Input input) = 0;

};