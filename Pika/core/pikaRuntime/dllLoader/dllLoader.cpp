#include "dllLoader.h"
#include "pikaConfig.h"

#ifdef PIKA_DEVELOPMENT

	#ifdef PIKA_WINDOWS
	
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	
	
	static HMODULE dllHand;
	
	
	//todo error reporting with strings
	bool pika::loadDll(std::filesystem::path path, 
		gameplayStart_t **gameplayStart, gameplayUpdate_t **gameplayUpdate, getContainersInfo_t **getContainersInfo
		,constructContainer_t **constructContainer, destructContainer_t **destructContainer)
	{
		path /= "pikaGameplay.dll";
	
		dllHand = LoadLibraryA(path.string().c_str());
	
		if (!dllHand) { return false; }
	
		*gameplayStart = (gameplayStart_t *)GetProcAddress(dllHand, "gameplayStart");
		*gameplayUpdate = (gameplayUpdate_t *)GetProcAddress(dllHand, "gameplayUpdate");
		*getContainersInfo = (getContainersInfo_t *)GetProcAddress(dllHand, "getContainersInfo");
		*constructContainer = (constructContainer_t *)GetProcAddress(dllHand, "constructContainer");
		*destructContainer = (destructContainer_t *)GetProcAddress(dllHand, "destructContainer");
	
		if (!gameplayStart) { return false; }
		if (!gameplayUpdate) { return false; }
		if (!getContainersInfo) { return false; }
		if (!constructContainer) { return false; }
		if (!destructContainer) { return false; }
	
		return	true;
	}
	
	
	#else
	#error "pika load dll works only on windows."
	#endif


#elif defined(PIKA_PRODUCTION)

	#include <dll/dllMain.h>

	bool pika::loadDll(std::filesystem::path path,
		gameplayStart_t **gameplayStart_, gameplayUpdate_t **gameplayUpdate_, getContainersInfo_t **getContainersInfo_
		,constructContainer_t **constructContainer_, destructContainer_t **destructContainer_)
	{
		
		*gameplayStart_ = gameplayStart;
		*gameplayUpdate_ = gameplayUpdate;
		*getContainersInfo_ = getContainersInfo;
		*constructContainer_ = constructContainer;
		*destructContainer_ = destructContainer;

		return	true;
	}


#endif

