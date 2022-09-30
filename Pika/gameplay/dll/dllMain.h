#pragma once
//////////////////////////////////////////
//dllMain.h
//Luta Vlad(c) 2022
//https://github.com/meemknight/PikaEngine
//////////////////////////////////////////

#include <pikaConfig.h>
#include <imgui.h>
#include <pikaImgui/pikaImgui.h>
#include <containers.h>
#include <containerInformation.h>
#include <vector>
#include <memoryArena/memoryArena.h>
#include <pikaAllocator/freeListAllocator.h>

PIKA_API void gameplayStart(pika::PikaContext &pikaContext);
PIKA_API void gameplayReload(pika::PikaContext &pikaContext);
PIKA_API void getContainersInfo(std::vector<pika::ContainerInformation> &info);
PIKA_API bool constructContainer(Container **c, pika::memory::MemoryArena *arena, const char *name);
PIKA_API void destructContainer(Container **c, pika::memory::MemoryArena *arena);
PIKA_API void bindAllocator(pika::memory::FreeListAllocator *arena);
PIKA_API void resetAllocator();
PIKA_API void dissableAllocators();
