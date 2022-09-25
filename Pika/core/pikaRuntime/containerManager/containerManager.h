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
			pika::DllLoader &dllLoader, pika::LogManager &logManager);

		void init();

		void update(
			pika::DllLoader &dllLoader,
			pika::Input input,
			float deltaTime,
			pika::WindowState windowState);

		bool destroyContainer(containerId_t id, pika::DllLoader &dllLoader,
			pika::LogManager &logManager);

		void destroyAllContainers(pika::DllLoader &dllLoader,
			pika::LogManager &logManager);

		containerId_t idCounter = 0;
	};




}