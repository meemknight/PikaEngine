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
#include <logs/log.h>
#include <sstream>

#define GAMEPLAYSTART(x) void x(pika::PikaContext &pikaContext)
typedef GAMEPLAYSTART(gameplayStart_t);
#undef GAMEPLAYSTART

#define GAMEPLAYRELOAD(x) void x(pika::PikaContext &pikaContext)
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

#define DISSABLEALLOCATORS(x) void x();
typedef DISSABLEALLOCATORS(dissableAllocators_t)
#undef DISSABLEALLOCATORS

#define GETCONSOLEBUFFER(x) std::streambuf * x();
typedef GETCONSOLEBUFFER(getConsoleBuffer_t)
#undef GETCONSOLEBUFFER



#ifdef PIKA_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace pika
{


struct LoadedDll
{
	gameplayStart_t *gameplayStart_ = {};
	gameplayReload_t *gameplayReload_ = {};
	getContainersInfo_t *getContainersInfo_ = {};
	constructContainer_t *constructContainer_ = {};
	destructContainer_t *destructContainer_ = {};
	bindAllocator_t *bindAllocator_ = {};
	resetAllocator_t *resetAllocator_ = {};
	dissableAllocators_t *dissableAllocators_ = {};
	getConsoleBuffer_t *getConsoleBuffer_ = {};

#ifdef PIKA_WINDOWS
	FILETIME filetime = {};
	HMODULE dllHand = {};
#endif

	int id = 0;

	bool loadDll(int id, pika::LogManager &logs);

	bool tryToloadDllUntillPossible(int id, pika::LogManager &logs, std::chrono::duration<long long> timeout = 
		std::chrono::seconds(0));

	void unloadDll();

	//no need to call since it is called in load dll function
	void getContainerInfoAndCheck(pika::LogManager &logs);

	bool shouldReloadDll();

	std::vector<pika::ContainerInformation> containerInfo;

	bool constructRuntimeContainer(pika::RuntimeContainer &c, const char *name);

	void bindAllocatorDllRealm(pika::memory::FreeListAllocator *allocator);

	void resetAllocatorDllRealm();

	bool checkIfDllIsOpenable();
};



};
