#include "dllLoader.h"
#include "pikaConfig.h"
#include <logs/assert.h>

#ifdef PIKA_DEVELOPMENT

	#ifdef PIKA_WINDOWS
	
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	
	

	static FILETIME getLastWriteFile(const char *name)
	{
		FILETIME time = {};

		WIN32_FILE_ATTRIBUTE_DATA Data = {};
		if (GetFileAttributesEx(name, GetFileExInfoStandard, &Data))
		{
			time = Data.ftLastWriteTime;
		}
		else
		{
			PIKA_PERMA_ASSERT(0, "Couldn't get file time");
		}

		return(time);
	}

	//todo error reporting with strings
	bool pika::DllLoader::loadDll(std::filesystem::path path)
	{
		p = path;

		std::filesystem::path originalDll = path / "pikaGameplay.dll";
		std::filesystem::path copyDll = path / "pikaGameplayCopy.dll";

		filetime = getLastWriteFile(originalDll.string().c_str());


		std::filesystem::copy(originalDll, copyDll, std::filesystem::copy_options::overwrite_existing);

		dllHand = LoadLibraryA(copyDll.string().c_str());

		if (!dllHand) { return false; }

		gameplayStart_ = (gameplayStart_t *)GetProcAddress(dllHand, "gameplayStart");
		gameplayReload_ = (gameplayReload_t *)GetProcAddress(dllHand, "gameplayReload");
		getContainersInfo_ = (getContainersInfo_t *)GetProcAddress(dllHand, "getContainersInfo");
		constructContainer_ = (constructContainer_t *)GetProcAddress(dllHand, "constructContainer");
		destructContainer_ = (destructContainer_t *)GetProcAddress(dllHand, "destructContainer");

		if (!gameplayStart_) { return false; }
		if (!gameplayReload_) { return false; }
		if (!getContainersInfo_) { return false; }
		if (!constructContainer_) { return false; }
		if (!destructContainer_) { return false; }

		return	true;
	}
	
	void pika::DllLoader::unloadDll()
	{
		FreeLibrary(dllHand);
	}

	bool pika::DllLoader::reloadDll()
	{
		std::filesystem::path originalDll = p / "pikaGameplay.dll";
		std::filesystem::path copyDll = p / "pikaGameplayCopy.dll";

		FILETIME newFiletime = getLastWriteFile(originalDll.string().c_str());

		if (CompareFileTime(&filetime, &newFiletime) != 0)
		{

			unloadDll();
			
			HANDLE fileCheck;
			while ((fileCheck = CreateFile(originalDll.string().c_str(),
				GENERIC_READ | GENERIC_WRITE, NULL, NULL,
				OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
			{
				//Wait till the dll can be oppened. It is possible that the compiler still keeps it busy.
			}
			CloseHandle(fileCheck);
			
			//try to load
			while (!loadDll(p)) {};

			return true;
		}

		return false;
	}
	
	#else
	#error "pika load dll works only on windows."
	#endif


#elif defined(PIKA_PRODUCTION)

	#include <dll/dllMain.h>

	bool pika::DllLoader::loadDll(std::filesystem::path path)
	{
		
		gameplayStart_ = gameplayStart;
		gameplayReload_ = gameplayReload;
		getContainersInfo_ = getContainersInfo;
		constructContainer_ = constructContainer;
		destructContainer_ = destructContainer;

		return	true;
	}

	bool pika::DllLoader::reloadDll()
	{
		return false;
	}

	void pika::DllLoader::unloadDll()
	{
	}


#endif

void pika::DllLoader::constructRuntimeContainer(RuntimeContainer &c, const char *name)
{
	constructContainer_(&c.pointer, &c.arena, name);
}

	
