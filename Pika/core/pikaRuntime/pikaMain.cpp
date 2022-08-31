#include <iostream>
#include <cstdio>
#include <filesystem>

#include <glad/glad.h>
#include <windowSystemm/window.h>


#include "assert/assert.h"
#include "dllLoader/dllLoader.h"
#include "pikaImgui/pikaImgui.h"

#include <pikaAllocator/memoryArena.h>
#include <runtimeContainer/runtimeContainer.h>

int main()
{

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



	RuntimeContainer container;
	container.arena.allocateStaticMemory(loadedContainers[0]);

	dllLoader.constructRuntimeContainer(container, "Gameplay");
	container.pointer->create();

	while (!window.shouldClose())
	{

		if (dllLoader.reloadDll())
		{
			dllLoader.gameplayReload_(window.context);
		}


		glClear(GL_COLOR_BUFFER_BIT);

		pika::imguiStartFrame(window.context);

		//gameplayUpdate(context);
		container.pointer->update(window.input);

		pika::imguiEndFrame(window.context);


		window.update();
	}



	return 0;
}