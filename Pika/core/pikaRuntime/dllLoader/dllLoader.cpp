#include "dllLoader.h"
#include "pikaConfig.h"

#ifdef PIKA_DEVELOPMENT

	#ifdef PIKA_WINDOWS
	
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	
	
	static HMODULE dllHand;

	//todo error reporting with strings
	bool pika::DllLoader::loadDll(std::filesystem::path path)
	{
		path /= "pikaGameplay.dll";

		dllHand = LoadLibraryA(path.string().c_str());

		if (!dllHand) { return false; }

		gameplayStart_ = (gameplayStart_t *)GetProcAddress(dllHand, "gameplayStart");
		gameplayUpdate_ = (gameplayUpdate_t *)GetProcAddress(dllHand, "gameplayUpdate");
		getContainersInfo_ = (getContainersInfo_t *)GetProcAddress(dllHand, "getContainersInfo");
		constructContainer_ = (constructContainer_t *)GetProcAddress(dllHand, "constructContainer");
		destructContainer_ = (destructContainer_t *)GetProcAddress(dllHand, "destructContainer");

		if (!gameplayStart_) { return false; }
		if (!gameplayUpdate_) { return false; }
		if (!getContainersInfo_) { return false; }
		if (!constructContainer_) { return false; }
		if (!destructContainer_) { return false; }

		return	true;
	}
	
	
	#else
	#error "pika load dll works only on windows."
	#endif


#elif defined(PIKA_PRODUCTION)

	#include <dll/dllMain.h>

	bool pika::DllLoader::loadDll(std::filesystem::path path)
	{
		
		gameplayStart_ = gameplayStart;
		gameplayUpdate_ = gameplayUpdate;
		getContainersInfo_ = getContainersInfo;
		constructContainer_ = constructContainer;
		destructContainer_ = destructContainer;

		return	true;
	}

	

#endif

	void pika::DllLoader::constructRuntimeContainer(RuntimeContainer &c, const char *name)
	{
		constructContainer_(&c.pointer, &c.arena, name);
	}
