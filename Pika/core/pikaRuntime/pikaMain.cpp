#include <iostream>
#include <cstdio>
#include <filesystem>

#include <glad/glad.h>
#include <windowSystemm/window.h>


#include "logs/assert.h"
#include "dllLoader/dllLoader.h"
#include "pikaImgui/pikaImgui.h"

#include <memoryArena/memoryArena.h>
#include <runtimeContainer/runtimeContainer.h>

#include <logs/log.h>
#include <logs/logWindow.h>

#include <editor/editor.h>
#include <shortcutApi/shortcutApi.h>
#include <globalAllocator/globalAllocator.h>

#include <containerManager/containerManager.h>

extern int n;

int main()
{

#pragma region init global variables stuff
	pika::initShortcutApi();
#pragma endregion


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

	std::vector<pika::ContainerInformation> loadedContainers;
	loadedContainers.reserve(100);
	//todo validate stuff
	dllLoader.getContainersInfo_(loadedContainers);
#pragma endregion

#pragma region container manager

	pika::ContainerManager containerManager;

	containerManager.init();

#pragma endregion


#pragma region shortcuts
	pika::ShortcutManager shortcutManager;
#pragma endregion

#pragma region editor
	pika::Editor editor;
	editor.init(shortcutManager);
#pragma endregion

	
	logs.log("test");


	containerManager.createContainer("Main level",
		loadedContainers[0], dllLoader);


	while (!window.shouldClose())
	{

	#pragma region reload dll
		if (dllLoader.reloadDll())
		{
			dllLoader.gameplayReload_(window.context);
		}
	#pragma endregion

	#pragma region start imgui
		pika::imguiStartFrame(window.context);
	#pragma endregion

	#pragma region clear screen
		glClear(GL_COLOR_BUFFER_BIT);
	#pragma endregion

	#pragma region editor stuff

		editor.update(window.input, shortcutManager, logs);


	#pragma endregion

		containerManager.update(dllLoader, window.input, window.deltaTime, window.windowState);


	#pragma region end imgui frame
		pika::imguiEndFrame(window.context);
	#pragma endregion


		window.update();
		shortcutManager.update(window.input);

		if (window.input.buttons[pika::Button::Q].released())
		{
			editor.pushNotificationManager.pushNotification("hello");
		}

	}

	containerManager.destroyAllContainers(dllLoader);

	return 0;
}