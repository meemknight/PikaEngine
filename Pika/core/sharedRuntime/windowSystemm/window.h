#pragma once
#include <glad/glad.h> //so we don't have conflicts
#include <GLFW/glfw3.h>
#include <pikaContext.h>
#include "input.h"
#include <chrono>

namespace pika
{

	
	struct WindowState
	{
		//might differ from framebuffer w, h
		int windowW = 0;
		int windowH = 0;

		//you should probably use this one
		int frameBufferW = 0;
		int frameBufferH = 0;

	};
	

	//this is not intended to have multiple instances in the program
	struct PikaWindow
	{
		pika::PikaContext context = {};


		//this is made to be passed to the user code
		//on live code editing this will be recorded every frame
		Input input = {};


		WindowState windowState = {};

		//this doesn't return error codes because it will do the asserts for you
		void create();

		void saveWindowPositions();

		bool shouldClose();

		void update();

		std::chrono::steady_clock::time_point timer = {};
	};



}