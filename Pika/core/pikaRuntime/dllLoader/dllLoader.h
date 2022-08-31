#pragma once
#include <filesystem>
#include <glad/glad.h> //used to not conflict with glfw
#include <GLFW/glfw3.h>
#include <pikaImgui/pikaImgui.h>
#include <dllLoader/containerInformation.h>
#include <vector>
#include <baseContainer.h>
#include <pikaAllocator/memoryArena.h>
#include <runtimeContainer/runtimeContainer.h>

#define GAMEPLAYSTART(x) void x(pika::PikaContext pikaContext)
typedef GAMEPLAYSTART(gameplayStart_t);
#undef GAMEPLAYSTART

#define GAMEPLAYRELOAD(x) void x(pika::PikaContext pikaContext)
typedef GAMEPLAYRELOAD(gameplayReload_t);
#undef GAMEPLAYRELOAD

#define GETCONTAINERSINFO(x) void x(std::vector<ContainerInformation> &info)
typedef GETCONTAINERSINFO(getContainersInfo_t);
#undef GETCONTAINERSINFO

#define CONSTRUCTCONTAINER(x) void x(Container **c, pika::memory::MemoryArena *arena, const char *name);
typedef CONSTRUCTCONTAINER(constructContainer_t);
#undef CONSTRUCTCONTAINER


#define DESTRUCTCONTAINER(x) void x(Container **c, pika::memory::MemoryArena *arena);
typedef DESTRUCTCONTAINER(destructContainer_t);
#undef DESTRUCTCONTAINER

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

	std::filesystem::path p = {};
	FILETIME filetime = {};
	HMODULE dllHand = {};

	bool loadDll(std::filesystem::path path);
	void constructRuntimeContainer(RuntimeContainer &c, const char *name);

	void unloadDll();

	//will check if the dll reloaded and reload it
	bool reloadDll();

};


};
