#include "dllMain.h"

#include <gl2d/gl2d.h>

#include <imgui.h>

#include "pikaImgui/pikaImgui.h"
#include <logs/assert.h>

#include "containers/pikaGameplay.h"
#include <containers.h>

#include <pikaAllocator/memoryArena.h>


#define PIKA_MAKE_CONTAINER_INFO(x) ContainerInformation(sizeof(x), #x)

PIKA_API void getContainersInfo(std::vector<ContainerInformation> &info)
{
	info.clear();
	info.push_back(PIKA_MAKE_CONTAINER_INFO(Gameplay));
}

PIKA_API void constructContainer(Container **c, pika::memory::MemoryArena *arena, const char *name)
{
	*c = getContainer(name, arena);
	PIKA_PERMA_ASSERT(*c, "coultn't create container(probably invalid name)");
	return;


}

PIKA_API void destructContainer(Container **c, pika::memory::MemoryArena *arena)
{
	//placement delete here
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