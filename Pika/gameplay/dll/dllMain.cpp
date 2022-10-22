#include "dllMain.h"

#include <gl2d/gl2d.h>

#include <imgui.h>

#include "pikaImgui/pikaImgui.h"
#include <logs/assert.h>

#include "containers/pikaGameplay.h"
#include <containers.h>

#include <memoryArena/memoryArena.h>
#include <globalAllocator/globalAllocator.h>

//todo use a global static array that can be accessed from other cpps and the macro will create an instance of a struct 
//that will push that container
#define PIKA_MAKE_CONTAINER_INFO(x) pika::ContainerInformation(sizeof(x), #x, x::containerInfo())

PIKA_API void getContainersInfo(std::vector<pika::ContainerInformation> &info)
{
	info.clear();
	info.push_back(PIKA_MAKE_CONTAINER_INFO(Gameplay));
	info.push_back(PIKA_MAKE_CONTAINER_INFO(ImmageViewer));
}

//this should not allocate memory
PIKA_API bool constructContainer(Container **c, pika::memory::MemoryArena *arena, const char *name)
{
	*c = getContainer(name, arena);
	return *c != 0;

}

PIKA_API void destructContainer(Container **c, pika::memory::MemoryArena *arena)
{
	//no need to call delete.
	(*c)->~Container();

}

PIKA_API void bindAllocator(pika::memory::FreeListAllocator *arena)
{
	pika::memory::setGlobalAllocator(arena);
}

PIKA_API void resetAllocator()
{
	pika::memory::setGlobalAllocatorToStandard();
}

PIKA_API void dissableAllocators()
{
	pika::memory::dissableAllocators();
}

//used to initialize libraries 
PIKA_API void gameplayStart(pika::PikaContext &pikaContext)
{
	pika::pikaImgui::setImguiAllocator(pikaContext.imguiAllocator);
	//pika::initShortcutApi(); //todo

	//todo user should have functions to specify this
#pragma region init stuff
#ifdef PIKA_DEVELOPMENT
	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad from dll");
	pika::pikaImgui::setImguiContext(pikaContext);
#endif

	gl2d::init();

#pragma endregion
}


static std::stringstream buff;
static std::streambuf *old = 0;

PIKA_API std::streambuf *getConsoleBuffer()
{
	if (old == nullptr)
	{
		old = std::cout.rdbuf(buff.rdbuf());
	}
	else 
	{
		std::cout.rdbuf(buff.rdbuf());
	}

	return buff.rdbuf();
}



//this won't be ever called in production so we can remove the code
PIKA_API void gameplayReload(pika::PikaContext &pikaContext)
{
#ifdef PIKA_DEVELOPMENT	

	pika::pikaImgui::setImguiAllocator(pikaContext.imguiAllocator); //todo check if really needed
	//pika::initShortcutApi();


	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad from dll");
	pika::pikaImgui::setImguiContext(pikaContext);

	gl2d::init();

#endif
}

#if PIKA_WINDOWS
#ifdef PIKA_DEVELOPMENT


#include <Windows.h>
//https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpvReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	// Initialize once for each new process.
	// Return FALSE to fail DLL load.
	break;

	case DLL_THREAD_ATTACH:
	// Do thread-specific initialization.
	break;

	case DLL_THREAD_DETACH:
	// Do thread-specific cleanup.
	break;

	case DLL_PROCESS_DETACH:

	if (lpvReserved != nullptr)
	{
		break; // do not do cleanup if process termination scenario
	}

	if (old)
	{
		std::cout.rdbuf(old);
	}

	// Perform any necessary cleanup.
	break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}


#endif
#endif
