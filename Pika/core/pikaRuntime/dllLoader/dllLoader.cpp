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
		testStart_t **testStart, testUpdate_t **testUpdate)
	{
		path /= "pikaGameplay.dll";
	
		dllHand = LoadLibraryA(path.string().c_str());
	
		if (!dllHand) { return false; }
	
		*testStart = (testStart_t *)GetProcAddress(dllHand, "testStart");
		*testUpdate = (testUpdate_t *)GetProcAddress(dllHand, "testUpdate");
	
		if (!testStart) { return false; }
		if (!testUpdate) { return false; }
	
		return	true;
	}
	
	
	#else
	#error "pika load dll works only on windows."
	#endif


#elif defined(PIKA_PRODUCTION)

	#include <dllMain.h>

	bool pika::loadDll(std::filesystem::path path,
		testStart_t **testStart_, testUpdate_t **testUpdate_)
	{
		
		*testStart_ = testStart;
		*testUpdate_ = testUpdate;
	
		return	true;
	}


#endif

