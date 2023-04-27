#include "dllLoader.h"
#include "pikaConfig.h"
#include <logs/assert.h>
#include <unordered_set>

static std::filesystem::path dllPath = std::filesystem::current_path();

#ifdef PIKA_DEVELOPMENT

	#ifdef PIKA_WINDOWS
	
	#define NOMINMAX
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
			return {};
		}

		return(time);
	}


	#else
	#error "pika load dll works only on windows, change configuration to pikaProduction"
	#endif


#endif


void pika::LoadedDll::reloadContainerExtensionsSupport()
{
#pragma region reload extension support
	{
		containerExtensionsSupport.clear();
		for (auto &c : containerInfo)
		{
			for (auto &e : c.containerStaticInfo.extensionsSuported)
			{
				if (containerExtensionsSupport.find(e.to_string()) == containerExtensionsSupport.end())
				{
					containerExtensionsSupport[e.to_string()] = {};
				}

				containerExtensionsSupport[e.to_string()].push_back(c.containerName);
			}
		}
	}
#pragma endregion
}

bool pika::LoadedDll::constructRuntimeContainer(RuntimeContainer &c, const char *name)
{
	PIKA_DEVELOPMENT_ONLY_ASSERT(constructContainer_ != nullptr, "dll not loaded");
	return constructContainer_(&c.pointer, &c.arena, name); //todo c.baseContainerName instead of name param
}



void pika::LoadedDll::bindAllocatorDllRealm(pika::memory::FreeListAllocator *allocator)
{
	PIKA_DEVELOPMENT_ONLY_ASSERT(bindAllocator_ != nullptr, "dll not loaded");
	bindAllocator_(allocator);
}

void pika::LoadedDll::resetAllocatorDllRealm()
{
	PIKA_DEVELOPMENT_ONLY_ASSERT(resetAllocator_ != nullptr, "dll not loaded");
	resetAllocator_();
}

void pika::LoadedDll::getContainerInfoAndCheck(pika::LogManager &logs)
{
	containerInfo.clear();
	containerInfo.reserve(100);

	//todo check for valid containers
	getContainersInfo_(containerInfo);

	std::unordered_set<std::string> uniqueNames = {};

	for (int i = 0; i < containerInfo.size(); i++)
	{
		auto signalError = [&](const char *e)
		{
			std::string l = e + containerInfo[i].containerName;
			logs.log(l.c_str(), pika::logError);

			containerInfo.erase(containerInfo.begin() + i);
			i--;
		};

		if (uniqueNames.find(containerInfo[i].containerName) == uniqueNames.end())
		{
			uniqueNames.insert(containerInfo[i].containerName);
		}
		else
		{
			signalError("Duplicate container name: ");
			continue;
		}

		if (containerInfo[i].containerStaticInfo._internalNotImplemented)
		{
			signalError("Container did not implement containerInfo function: ");
			continue;
		}

		if (containerInfo[i].containerStaticInfo.defaultHeapMemorySize < 100)
		{
			signalError("Too little heap memory for container: ");
			continue;
		}

	}
}


#ifdef PIKA_DEVELOPMENT


bool pika::LoadedDll::loadDll(int id, pika::LogManager &logs)
{
	unloadDll();


	std::filesystem::path originalDll = dllPath / "pikaGameplay.dll";
	std::filesystem::path copyDll = dllPath / ( "pikaGameplayCopy" + std::to_string(id) + ".dll");

	filetime = getLastWriteFile(originalDll.string().c_str());

	if (filetime.dwLowDateTime == FILETIME().dwLowDateTime
		&&
		filetime.dwHighDateTime == FILETIME().dwHighDateTime
		) { return false; }


	//std::filesystem::copy(originalDll, copyDll, std::filesystem::copy_options::overwrite_existing);
	if (!CopyFile(originalDll.string().c_str(), copyDll.string().c_str(), false) ) { return false; }


	dllHand = LoadLibraryA(copyDll.string().c_str());

	if (!dllHand) { return false; }

	gameplayStart_ = (gameplayStart_t *)GetProcAddress(dllHand, "gameplayStart");
	gameplayReload_ = (gameplayReload_t *)GetProcAddress(dllHand, "gameplayReload");
	getContainersInfo_ = (getContainersInfo_t *)GetProcAddress(dllHand, "getContainersInfo");
	constructContainer_ = (constructContainer_t *)GetProcAddress(dllHand, "constructContainer");
	destructContainer_ = (destructContainer_t *)GetProcAddress(dllHand, "destructContainer");
	bindAllocator_ = (bindAllocator_t *)GetProcAddress(dllHand, "bindAllocator");
	resetAllocator_ = (resetAllocator_t *)GetProcAddress(dllHand, "resetAllocator");
	dissableAllocators_ = (dissableAllocators_t *)GetProcAddress(dllHand, "dissableAllocators");

	if (!gameplayStart_) { return false; }
	if (!gameplayReload_) { return false; }
	if (!getContainersInfo_) { return false; }
	if (!constructContainer_) { return false; }
	if (!destructContainer_) { return false; }
	if (!bindAllocator_) { return false; }
	if (!resetAllocator_) { return false; }
	if (!dissableAllocators_) { return false; }

	//get container info
	getContainerInfoAndCheck(logs);
	this->id = id;

	reloadContainerExtensionsSupport();

	return	true;
}

bool pika::LoadedDll::checkIfDllIsOpenable()
{
	HANDLE fileCheck = {};
	fileCheck = CreateFile((dllPath / "pikaGameplay.dll").string().c_str(),
		GENERIC_READ | GENERIC_WRITE, NULL, NULL,
		OPEN_EXISTING, 0, NULL);

	if (fileCheck == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		CloseHandle(fileCheck);
		return true;
	}
}

bool pika::LoadedDll::tryToloadDllUntillPossible(int id, pika::LogManager &logs,
	std::chrono::duration<long long> timeout)
{
	auto startTime = std::chrono::steady_clock::now();

	while (!checkIfDllIsOpenable())
	{
		if (timeout != std::chrono::seconds(0))
		{
			if (std::chrono::steady_clock::now() > startTime + timeout)
			{
				return false;	//timeout
			}
		}
		//Wait till the dll can be oppened. It is possible that the compiler still keeps it busy.
	}

	unloadDll();


	//try to load (we loop since it is still possible that windows thinks that the dll is not available yet)
	while (!loadDll(id, logs)) 
	{
		if (timeout != std::chrono::seconds(0))
		{
			if (std::chrono::steady_clock::now() > startTime + timeout)
			{
				return false;	//timeout
			}
		}
	};
	return true;
}

void pika::LoadedDll::unloadDll()
{
	if (dllHand == 0) { return; }

	//dissableAllocators_();

	resetAllocatorDllRealm();

	FreeLibrary(dllHand);
	dllHand = {};
	filetime = {};
	containerInfo.clear();
}

bool pika::LoadedDll::shouldReloadDll()
{
	if (dllHand == 0) { return 0; }

	std::filesystem::path originalDll = dllPath / "pikaGameplay.dll";

	FILETIME newFiletime = getLastWriteFile(originalDll.string().c_str());

	if (filetime.dwLowDateTime == FILETIME().dwLowDateTime
		&&
		filetime.dwHighDateTime == FILETIME().dwHighDateTime
		)
	{
		return false;
	}

	return (CompareFileTime(&filetime, &newFiletime) != 0);
}



#elif defined(PIKA_PRODUCTION)

#include <dll/dllMain.h>

bool pika::LoadedDll::loadDll(int id, pika::LogManager &logs)
{

	gameplayStart_ = gameplayStart;
	gameplayReload_ = gameplayReload;
	getContainersInfo_ = getContainersInfo;
	constructContainer_ = constructContainer;
	destructContainer_ = destructContainer;
	bindAllocator_ = bindAllocator;
	resetAllocator_ = resetAllocator;
	dissableAllocators_ = dissableAllocators;
	getContainerInfoAndCheck(logs);
	this->id = id;

	reloadContainerExtensionsSupport();

	return	true;
}


bool pika::LoadedDll::tryToloadDllUntillPossible(int id, pika::LogManager &logs,
	std::chrono::duration<long long> timeout)
{
	return loadDll(id, logs);
}

void pika::LoadedDll::unloadDll()
{
	containerInfo.clear();
}

bool pika::LoadedDll::shouldReloadDll()
{
	return false;
}

bool pika::LoadedDll::checkIfDllIsOpenable()
{
	return true;
}

#endif

