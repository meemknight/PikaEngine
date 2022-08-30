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

#define GAMEPLAYUPDATE(x) void x(pika::PikaContext pikaContext)
typedef GAMEPLAYUPDATE(gameplayUpdate_t);
#undef GAMEPLAYUPDATE

#define GETCONTAINERSINFO(x) void x(std::vector<ContainerInformation> &info)
typedef GETCONTAINERSINFO(getContainersInfo_t);
#undef GETCONTAINERSINFO

#define CONSTRUCTCONTAINER(x) void x(Container **c, pika::memory::MemoryArena *arena, const char *name);
typedef CONSTRUCTCONTAINER(constructContainer_t);
#undef CONSTRUCTCONTAINER


#define DESTRUCTCONTAINER(x) void x(Container **c, pika::memory::MemoryArena *arena);
typedef DESTRUCTCONTAINER(destructContainer_t);
#undef DESTRUCTCONTAINER



namespace pika
{

//this will have only one instance open at a time for now
struct DllLoader
{
	gameplayStart_t *gameplayStart_ = {};
	gameplayUpdate_t *gameplayUpdate_ = {};
	getContainersInfo_t *getContainersInfo_ = {};
	constructContainer_t *constructContainer_ = {};
	destructContainer_t *destructContainer_ = {};

	bool loadDll(std::filesystem::path path);
	void constructRuntimeContainer(RuntimeContainer &c, const char *name);

};


};
