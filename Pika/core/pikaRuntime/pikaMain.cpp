#include <iostream>
#include <cstdio>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include "assert/assert.h"
#include "dllLoader/dllLoader.h"
#include "pikaImgui/pikaImgui.h"

#include <pikaAllocator/memoryArena.h>

int main()
{
	std::filesystem::path currentPath = std::filesystem::current_path();
	
	
	gameplayStart_t *gameplayStart = {};
	gameplayUpdate_t *gameplayUpdate = {};
	getContainersInfo_t *getContainersInfo = {};
	constructContainer_t *constructContainer = {};
	destructContainer_t *destrContainer = {};

	PIKA_PERMA_ASSERT(pika::loadDll(currentPath, &gameplayStart, &gameplayUpdate, &getContainersInfo,
		&constructContainer, &destrContainer), "Couldn't load dll");
	
	PIKA_PERMA_ASSERT(glfwInit(), "Problem initializing glfw");

	//glfwSetErrorCallback(error_callback); todo

	pika::PikaContext context = {};

	context.wind = glfwCreateWindow(640, 480, "Pika", NULL, NULL);
	if (!context.wind)
	{
		std::cout << "problem initializing window";
	}

	glfwMakeContextCurrent(context.wind);

	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad");

	context.ImGuiContext = pika::initImgui(context);

	context.glfwMakeContextCurrentPtr = glfwMakeContextCurrent;

	std::vector<ContainerInformation> loadedContainers;
	//todo validate stuff
	getContainersInfo(loadedContainers);
	pika::memory::MemoryArena arena = {};

	arena.containerStructMemory.size = loadedContainers[0].containerStructBaseSize;
	arena.containerStructMemory.block = malloc(loadedContainers[0].containerStructBaseSize);

	gameplayStart(context);

	Container *gameCode = 0;
	constructContainer(&gameCode, &arena, "Gameplay");
	gameCode->create();

	while (!glfwWindowShouldClose(context.wind))
	{

		glClear(GL_COLOR_BUFFER_BIT);

		pika::imguiStartFrame(context);

		//gameplayUpdate(context);
		gameCode->update();

		pika::imguiEndFrame(context);



		glfwPollEvents();
		glfwSwapBuffers(context.wind);
	}



	return 0;
}