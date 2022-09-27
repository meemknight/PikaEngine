#pragma once
#include <logs/log.h>
#include <runtimeContainer/runtimeContainer.h>

#include <unordered_map>
#include <string>

#include <dllLoader/dllLoader.h>

#include <windowSystemm/input.h>
#include <windowSystemm/window.h>

namespace pika
{
	using containerId_t = unsigned int;
	
	struct ContainerManager
	{


		std::unordered_map<containerId_t, pika::RuntimeContainer> runningContainers;

		containerId_t createContainer(
			pika::ContainerInformation containerInformation,
			pika::LoadedDll &loadedDll, pika::LogManager &logManager);

		void init();

		void update(
			pika::LoadedDll &loadedDll,
			pika::PikaWindow &window,
			pika::LogManager &logs);

		bool destroyContainer(containerId_t id, pika::LoadedDll &loadedDll,
			pika::LogManager &logManager);

		//same as destroy container but doesn't call user destructors
		bool forceTerminateContainer(containerId_t id, pika::LoadedDll &loadedDll,
			pika::LogManager &logManager);

		void destroyAllContainers(pika::LoadedDll &loadedDll,
			pika::LogManager &logManager);

		containerId_t idCounter = 0;

	};




}