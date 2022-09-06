#include <iostream>
#include <cstdio>
#include <filesystem>

#include <glad/glad.h>
#include <windowSystemm/window.h>


#include "logs/assert.h"
#include "dllLoader/dllLoader.h"
#include "pikaImgui/pikaImgui.h"

#include <pikaAllocator/memoryArena.h>
#include <runtimeContainer/runtimeContainer.h>

#include <logs/log.h>

int main()
{

#pragma region log
	pika::LogManager logs;
	logs.init("logs.txt");
#pragma endregion

#pragma region load dll
	std::filesystem::path currentPath = std::filesystem::current_path();
	pika::DllLoader dllLoader;
	PIKA_PERMA_ASSERT(dllLoader.loadDll(currentPath), "Couldn't load dll");
#pragma endregion
	
#pragma region init window opengl imgui and context
	PIKA_PERMA_ASSERT(glfwInit(), "Problem initializing glfw");
	//glfwSetErrorCallback(error_callback); todo
	pika::PikaWindow window = {};
	window.create();

	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad");

	pika::initImgui(window.context);

	window.context.glfwMakeContextCurrentPtr = glfwMakeContextCurrent;
#pragma endregion

#pragma region init dll reaml
	dllLoader.gameplayStart_(window.context);

	std::vector<ContainerInformation> loadedContainers;
	//todo validate stuff
	dllLoader.getContainersInfo_(loadedContainers);
#pragma endregion

	logs.log("test");

	RuntimeContainer container;
	container.arena.allocateStaticMemory(loadedContainers[0]); //this just allocates the memory

	dllLoader.constructRuntimeContainer(container, "Gameplay"); //this calls the constructors
	container.pointer->create();	//this calls create()

	while (!window.shouldClose())
	{


		if (dllLoader.reloadDll())
		{
			dllLoader.gameplayReload_(window.context);
		}


		glClear(GL_COLOR_BUFFER_BIT);

		pika::imguiStartFrame(window.context);

		//gameplayUpdate(context);
		container.pointer->update(window.input, window.deltaTime, window.windowState);

		pika::imguiEndFrame(window.context);


		window.update();
	}



	return 0;
}