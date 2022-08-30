#pragma once
#include <GLFW/glfw3.h>
#include <pikaContext.h>


namespace pika
{

	//this is not intended to have multiple instances in the program
	struct PikaWindow
	{
		pika::PikaContext context = {};

		//this doesn't return error codes because it will do the asserts for you
		void create();

		bool shouldClose();

		void update();

	};



}