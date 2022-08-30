#pragma once
#include <pikaConfig.h>
#include <imgui.h>
#include <pikaImgui/pikaImgui.h>
#include <containers.h>
#include <dllLoader/containerInformation.h>
#include <vector>
#include <dllLoader/dllLoader.h>
#include <pikaAllocator/memoryArena.h>	

PIKA_API void gameplayStart(pika::PikaContext pikaContext);
PIKA_API void gameplayUpdate(pika::PikaContext pikaContext);
PIKA_API void getContainersInfo(std::vector<ContainerInformation> &info);
PIKA_API void constructContainer(Container **c, pika::memory::MemoryArena *arena, const char *name);
PIKA_API void destructContainer(Container **c, pika::memory::MemoryArena *arena);