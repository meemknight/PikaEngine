#pragma once
#include <filesystem>
#include <glad/glad.h> //used to not conflict with glfw
#include <GLFW/glfw3.h>
#include <pikaImgui/pikaImgui.h>
#include <containerInformation.h>
#include <vector>
#include <baseContainer.h>
#include <memoryArena/memoryArena.h>
#include <runtimeContainer/runtimeContainer.h>

#define GAMEPLAYSTART(x) void x(pika::PikaContext pikaContext)
typedef GAMEPLAYSTART(gameplayStart_t);
#undef GAMEPLAYSTART

#define GAMEPLAYRELOAD(x) void x(pika::PikaContext pikaContext)
typedef GAMEPLAYRELOAD(gameplayReload_t);
#undef GAMEPLAYRELOAD

#define GETCONTAINERSINFO(x) void x(std::vector<pika::ContainerInformation> &info)
typedef GETCONTAINERSINFO(getContainersInfo_t);
#undef GETCONTAINERSINFO

#define CONSTRUCTCONTAINER(x) bool x(Container **c, pika::memory::MemoryArena *arena, const char *name);
typedef CONSTRUCTCONTAINER(constructContainer_t);
#undef CONSTRUCTCONTAINER

#define DESTRUCTCONTAINER(x) void x(Container **c, pika::memory::MemoryArena *arena);
typedef DESTRUCTCONTAINER(destructContainer_t);
#undef DESTRUCTCONTAINER

#define BINDALLOCATOR(x) void x(pika::memory::FreeListAllocator *arena);
typedef BINDALLOCATOR(bindAllocator_t);
#undef BINDALLOCATOR

#define RESETALLOCATOR(x) void x();
typedef RESETALLOCATOR(resetAllocator_t)
#undef RESETALLOCATOR

//todo remove windows include 
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace pika
{

//this will have only one instance open at a time for now
struct DllLoader
{
	gameplayStart_t *gameplayStart_ = {};
	gameplayReload_t *gameplayReload_ = {};
	getContainersInfo_t *getContainersInfo_ = {};
	constructContainer_t *constructContainer_ = {};
	destructContainer_t *destructContainer_ = {};
	bindAllocator_t *bindAllocator_ = {};
	resetAllocator_t *resetAllocator_ = {};

	std::filesystem::path p = {};
	FILETIME filetime = {};
	HMODULE dllHand = {};

	bool loadDll(std::filesystem::path path);
	bool constructRuntimeContainer(pika::RuntimeContainer &c, const char *name);
	void bindAllocatorDllRealm(pika::memory::FreeListAllocator *allocator);
	void resetAllocatorDllRealm();

	void unloadDll();

	//will check if the dll reloaded and reload it
	bool reloadDll();

};


};
