#pragma once
#include <windowSystemm/window.h>
#include <iostream>
#include <pikaOptional.h>
#include <string>
#include <pikaAllocator/freeListAllocator.h>
#include <staticVector.h>
#include <pikaGL/frameBuffer.h>
#include <pikaConsoleManager/pikaConsoleWindow.h>
#include <globalAllocator/globalAllocator.h>
#include <fstream>
#include <staticString.h>
#include <vector>
#include <stringManipulation/stringManipulation.h>


#define READENTIREFILE(x) bool x(const char* name, void* buffer, size_t size)
typedef READENTIREFILE(readEntireFile_t);
#undef READENTIREFILE

#define GETFILESIZE(x) bool x(const char* name, size_t &size)
typedef GETFILESIZE(getFileSize_t);
#undef GETFILESIZE

static constexpr size_t MaxAllocatorsCount = 64;

struct CreateContainerInfo
{
	pika::StaticString<257> containerName = {};
	pika::StaticString<257> cmdArgs = {};
};

//this is passed by the engine. You should not modify the data
//this is also used by the engine to give you acces to some io functions
struct RequestedContainerInfo
{
	pika::memory::FreeListAllocator *mainAllocator = {};
	pika::StaticVector<pika::memory::FreeListAllocator, MaxAllocatorsCount> *bonusAllocators = {};

	//readEntireFile_t *readEntireFilePointer = {};
	//getFileSize_t *getFileSizePointer = {};

	pika::GL::PikaFramebuffer requestedFBO = {};

	int requestedImguiIds = 0;
	int imguiTotalRequestedIds = 0;
	int pushImguiIdForMe = 0;

	pika::ConsoleWindow *consoleWindow = nullptr;

	pika::LogManager *logManager = nullptr;

	void setMousePositionRelevantToWindow(int x, int y) 
	{
		if (internal.mainWindow)
		{
			internal.setCursorPosFunc(internal.window, x, y);
		}
		else
		{
			int mainX = 0;
			int mainY = 0;
			internal.getWindowPosFunc(internal.window, &mainX, &mainY);
			//internal.setCursorPosFunc(internal.window, x + internal.windowPosX - mainX, y + internal.windowPosY - mainY);
			internal.setCursorPosFunc(internal.window, x + internal.windowPosX - mainX + 8, y + internal.windowPosY - mainY + 28);
		}
	};

	void setFpsCursor()
	{
		PIKA_DEVELOPMENT_ONLY_ASSERT(internal.setInputModeFunc, "missing setInputModeFunc func");
	
		internal.setInputModeFunc(internal.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void setNormalCursor()
	{
		PIKA_DEVELOPMENT_ONLY_ASSERT(internal.setInputModeFunc, "missing setInputModeFunc func");

		internal.setInputModeFunc(internal.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	struct
	{
		int windowPosX = 0;
		int windowPosY = 0;
		GLFWwindow *window = 0;
		bool mainWindow = 0;
		decltype(glfwSetCursorPos) *setCursorPosFunc = nullptr;
		decltype(glfwGetWindowPos) *getWindowPosFunc = nullptr;
		decltype(glfwSetInputMode) *setInputModeFunc = nullptr;

		std::vector<CreateContainerInfo> *containersToCreate;

	}internal;

	//resets global allocator!
	void createContainer(std::string containerName, std::string cmdArgs = "")
	{
		PIKA_DEVELOPMENT_ONLY_ASSERT(internal.containersToCreate, "missing containersToCreate pointer");
		CreateContainerInfo info;

		if (containerName.size() >= info.containerName.MAX_SIZE-1) { return; }
		if (cmdArgs.size() >= info.cmdArgs.MAX_SIZE-1) { return; }

		info.containerName = {containerName.c_str()};
		info.cmdArgs = {cmdArgs.c_str()};

		pika::memory::pushCustomAllocatorsToStandard();
		internal.containersToCreate->push_back(info);
		pika::memory::popCustomAllocatorsToStandard();

	}

	bool log(const char* l, int type)
	{
		//do not allocate memory here!
		//log is from core realm

		if (!logManager) { return false; }
		logManager->log(l, type);
	}

	//returns true if succeded (can return false if console is disabeled)
	bool consoleWrite(const char* c)
	{
		//do not allocate memory here!
		//console window is from core realm

		if (!consoleWindow) { return false; }
		consoleWindow->write(c);
		return true;
	}

	bool consoleWrite(std::string &s)
	{
		return consoleWrite(s.c_str());
	}

	bool writeEntireFile(const char *name, const std::string &content)
	{
		std::ofstream f(name);

		if (!f.is_open()) { return 0; }

		f.write(content.c_str(), content.size());
		
		f.close();
		return 1;
	}

	bool writeEntireFileBinary(const char *name, void *data, size_t s)
	{
		std::ofstream f(name, std::ios::binary | std::ios::out);

		if (!f.is_open()) { return 0; }

		f.write((char*)data, s);

		f.close();
		return 1;
	}

	bool appendFileBinary(std::string_view name, void *data, size_t s)
	{
		std::ofstream f(name, std::ios::binary | std::ios::out | std::ios::app);

		if (!f.is_open()) { return 0; }

		f.write((char *)data, s);

		f.close();
		return 1;
	}

	bool readEntireFileBinary(std::string_view name, std::vector<char> &data)
	{
		size_t s = 0;
		data.clear();

		if (!getFileSizeBinary(name, s)) { return 0; }

		data.reserve(s);

		return readEntireFileBinary(name, data.data(), s);
	}

	bool readEntireFile(std::string_view name, std::string &data)
	{
		size_t s = 0;
		data.clear();

		if (!getFileSize(name, s)) { return 0; }

		data.resize(s);

		return readEntireFile(name, data.data(), s);
	}

	std::string readEntireFileBinaryAsAString(std::string_view name)
	{
		std::string rez;
		size_t s = 0;
		bool succeed = getFileSizeBinary(name, s);
		if (!succeed) { return rez; }

		rez.resize(s + 1);

		pika::memory::pushCustomAllocatorsToStandard();
		{
			std::ifstream f(name, std::ios::binary);
			if (!f.is_open())
			{
				succeed = false;
			}
			else
			{
				f.read((char *)rez.data(), s);
				f.close();
			}
		}
		pika::memory::popCustomAllocatorsToStandard();

		if (!succeed) { return ""; }
		return rez;
	}

	bool readEntireFileBinary(std::string_view name, void *buffer, size_t size, size_t from = 0)
	{
		//PIKA_DEVELOPMENT_ONLY_ASSERT(readEntireFilePointer, "read entire file pointer not assigned");
		bool success = true;

		pika::memory::pushCustomAllocatorsToStandard();
		{
			std::ifstream f(name, std::ios::binary);

			if (!f.is_open())
			{
				success = false;
			}
			else
			{
				f.seekg(from);
				f.read((char*)buffer, size);
				f.close();
			}
		}
		pika::memory::popCustomAllocatorsToStandard();

		return success;
	}

	bool readEntireFile(std::string_view name, void *buffer, size_t size)
	{
		//PIKA_DEVELOPMENT_ONLY_ASSERT(readEntireFilePointer, "read entire file pointer not assigned");
		bool success = true;

		pika::memory::pushCustomAllocatorsToStandard();
		{
			std::ifstream f(name);

			if (!f.is_open())
			{
				success = false;
			}
			else
			{
				f.read((char *)buffer, size);
				f.close();
			}
		}
		pika::memory::popCustomAllocatorsToStandard();

		return success;
	}

	bool getFileSizeBinary(std::string_view name, size_t &size)
	{
		//PIKA_DEVELOPMENT_ONLY_ASSERT(getFileSizePointer, "get file size pointer not assigned");

		bool success = true;
		size = 0;

		//todo push pop allocator or use that pointer thing (and don't forget to only use explicit allocators calls or sthing)
		pika::memory::pushCustomAllocatorsToStandard();
		{
			std::ifstream f(name, std::ifstream::ate | std::ifstream::binary);
			if (!f.is_open())
			{
				success = false;
			}
			else
			{
				size = f.tellg();
				f.close();
			}
		}
		pika::memory::popCustomAllocatorsToStandard();

		return size;
	}

	bool getFileSize(std::string_view name, size_t &size)
	{
		bool success = true;
		size = 0;
		pika::memory::pushCustomAllocatorsToStandard();
		{
			std::ifstream f(name, std::ifstream::ate);
			if (!f.is_open())
			{
				success = false;
			}
			else
			{
				size = f.tellg();
				f.close();
			}
		}
		pika::memory::popCustomAllocatorsToStandard();

		return success;
	}
};


struct ContainerStaticInfo
{

	//this is the main heap allocator memory size
	size_t defaultHeapMemorySize = 0;
	

	//this will use the global allocator. you won't be able to use input recording or snapshots, and the 
	//memory leak protection won't be possible.
	bool useDefaultAllocator = 0;


	pika::StaticVector<size_t, MaxAllocatorsCount> bonusAllocators = {};

	//add file extensions here so that the engine knows that your container can open them.
	pika::StaticVector<pika::StaticString<16>, 16> extensionsSuported = {};

	//the engine will create a new window for your container and give you the fbo to bind to
	//in release that fbo will just be the default framebuffer
	bool requestImguiFbo = 0;

	unsigned int requestImguiIds = 0;

	bool andInputWithWindowHasFocus = 1;
	bool andInputWithWindowHasFocusLastFrame = 1;
	bool openOnApplicationStartup = 0;
	bool pushAnImguiIdForMe = 0;

	bool _internalNotImplemented = 0;

	bool operator==(const ContainerStaticInfo &other)
	{
		if (this == &other) { return true; }

		return
			this->defaultHeapMemorySize == other.defaultHeapMemorySize &&
			this->bonusAllocators == other.bonusAllocators &&
			this->_internalNotImplemented == other._internalNotImplemented &&
			this->requestImguiFbo == other.requestImguiFbo &&
			this->requestImguiIds == other.requestImguiIds &&
			this->useDefaultAllocator == other.useDefaultAllocator &&
			this->andInputWithWindowHasFocus == other.andInputWithWindowHasFocus &&
			this->andInputWithWindowHasFocusLastFrame == other.andInputWithWindowHasFocusLastFrame &&
			this->pushAnImguiIdForMe == other.pushAnImguiIdForMe &&
			this->openOnApplicationStartup == other.openOnApplicationStartup;
	}

	bool operator!=(const ContainerStaticInfo &other)
	{
		return !(*this == other);
	}


};


struct Container
{

	//this is used to give to the engine basic information about your container.
	//this function should be pure
	//this function should not allocate memory
	//this should not be dependent on anything that is called on create or library initialization
	static ContainerStaticInfo containerInfo() { ContainerStaticInfo c; c._internalNotImplemented = true; return c; };
	

	virtual bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument) = 0;

	virtual bool update(
		pika::Input input, 
		pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo) = 0;

	virtual void destruct() {};

	virtual ~Container() {};

};