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

	
	struct ContainerManager
	{

		std::unordered_map<std::string, pika::RuntimeContainer> runningContainers;

		bool createContainer(
			std::string name, pika::ContainerInformation containerInformation,
			pika::DllLoader &dllLoader, pika::LogManager &logManager);

		void init();

		void update(
			pika::DllLoader &dllLoader,
			pika::Input input,
			float deltaTime,
			pika::WindowState windowState);

		bool destroyContainer(std::string name, pika::DllLoader &dllLoader,
			pika::LogManager &logManager);

		void destroyAllContainers(pika::DllLoader &dllLoader,
			pika::LogManager &logManager);
	};




}