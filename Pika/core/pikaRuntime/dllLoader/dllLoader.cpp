#include "dllLoader.h"
#include "pikaConfig.h"

#ifdef PIKA_DEVELOPMENT

	#ifdef PIKA_WINDOWS
	
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	
	
	static HMODULE dllHand;
	
	
	//todo error reporting with strings
	bool pika::loadDll(std::filesystem::path path, testPrint_t **testPrint)
	{
		path /= "pikaGameplay.dll";
	
		dllHand = LoadLibraryA(path.string().c_str());
	
		if (!dllHand) { return false; }
	
		*testPrint = (testPrint_t *)GetProcAddress(dllHand, "testPrint");
	
		if (!testPrint) { return false; }
	
	
		return	true;
	}
	
	
	#else
	#error "pika load dll works only on windows."
	#endif


#elif defined(PIKA_PRODUCTION)

	#include <dllMain.h>

	bool pika::loadDll(std::filesystem::path path, testPrint_t **testPrint_)
	{
		
		*testPrint_ = testPrint;
	
		return	true;
	}


#endif

