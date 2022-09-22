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


//used to initialize libraries 
PIKA_API void gameplayStart(pika::PikaContext pikaContext)
{
	//todo user should have functions to specify this
#pragma region init stuff
#ifdef PIKA_DEVELOPMENT
	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad from dll");
	pika::setImguiContext(pikaContext);
#endif

	gl2d::init();

#pragma endregion
}


//this won't be ever called in production so we can remove the code
PIKA_API void gameplayReload(pika::PikaContext pikaContext)
{
#ifdef PIKA_DEVELOPMENT	

	PIKA_PERMA_ASSERT(gladLoadGL(), "Problem initializing glad from dll");
	pika::setImguiContext(pikaContext);

	gl2d::init();

#endif
}