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
#include <staticVector.h>

int main()
{

#pragma region init global variables stuff
	pika::initShortcutApi();
#pragma endregion

#pragma region log
	pika::LogManager logs;
	logs.init(pika::LogManager::DefaultLogFile);

#pragma endregion

#pragma region load dll
	std::filesystem::path currentPath = std::filesystem::current_path();
	pika::LoadedDll loadedDll;
	PIKA_PERMA_ASSERT(loadedDll.loadDll(0, logs), "Couldn't load dll");
#pragma endregion
	
#pragma region pika imgui id manager
	pika::ImGuiIdsManager imguiIdsManager;
#pragma endregion

#pragma region push notification manager
#if !(defined(PIKA_PRODUCTION) && PIKA_REMOVE_PUSH_NOTIFICATION_IN_PRODUCTION)
	pika::PushNotificationManager pushNotificationManager; 
	pushNotificationManager.init();
	logs.pushNotificationManager = &pushNotificationManager;
#endif
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

#pragma region container manager

	pika::ContainerManager containerManager;

	containerManager.init();

#pragma endregion

#pragma region init dll reaml
	loadedDll.gameplayStart_(window.context);

	
#pragma endregion




#pragma region shortcuts
	pika::ShortcutManager shortcutManager;
#pragma endregion

#pragma region editor
#if !(defined(PIKA_PRODUCTION) && PIKA_REMOVE_EDITOR_IN_PRODUCATION)
	pika::Editor editor; //todo remove editor in production
	editor.init(shortcutManager);
#endif
#pragma endregion

	
	auto container = containerManager.createContainer
		(loadedDll.containerInfo[0], loadedDll, logs);

	while (!window.shouldClose())
	{


	#pragma region start imgui
		pika::imguiStartFrame(window.context);
	#pragma endregion

	#pragma region clear screen
		glClear(GL_COLOR_BUFFER_BIT);
	#pragma endregion

	#pragma region editor stuff
	#if !(defined(PIKA_PRODUCTION) && PIKA_REMOVE_EDITOR_IN_PRODUCATION)
		editor.update(window.input, shortcutManager, logs, 
			pushNotificationManager);
	#endif
	#pragma endregion

	#pragma region push notification
	#if !(defined(PIKA_PRODUCTION) && PIKA_REMOVE_PUSH_NOTIFICATION_IN_PRODUCTION)
		static bool pushNoticicationOpen = true;
		pushNotificationManager.update(pushNoticicationOpen);
	#endif
	#pragma endregion

	#pragma region container manager
		containerManager.update(loadedDll, window, logs);
	#pragma endregion

	#pragma region end imgui frame
		pika::imguiEndFrame(window.context);
	#pragma endregion

	#pragma region window update
		window.update();
	#pragma endregion

	#pragma region shortcut manager update
		shortcutManager.update(window.input);
	#pragma endregion
	
	}

	containerManager.destroyAllContainers(loadedDll, logs);

	return 0;
}