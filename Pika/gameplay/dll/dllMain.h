#pragma once
#include <pikaConfig.h>
#include <imgui.h>
#include <pikaImgui/pikaImgui.h>
#include <containers.h>
#include <containerInformation.h>
#include <vector>
#include <pikaAllocator/memoryArena.h>	

PIKA_API void gameplayStart(pika::PikaContext pikaContext);
PIKA_API void gameplayReload(pika::PikaContext pikaContext);
PIKA_API void getContainersInfo(std::vector<ContainerInformation> &info);
PIKA_API void constructContainer(Container **c, pika::memory::MemoryArena *arena, const char *name);
PIKA_API void destructContainer(Container **c, pika::memory::MemoryArena *arena);