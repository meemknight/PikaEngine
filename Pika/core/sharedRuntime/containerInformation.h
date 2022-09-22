#pragma once
#include <string>
#include <baseContainer.h>

namespace pika
{

struct ContainerInformation
{
	ContainerInformation() {};
	ContainerInformation(
		size_t containerStructBaseSize,
		const char *containerName,
		const ContainerStaticInfo& containerStaticInfo):
		containerStructBaseSize(containerStructBaseSize), containerName(containerName),
		containerStaticInfo(containerStaticInfo)
	{
	};

	size_t containerStructBaseSize = 0; //static memory
	std::string containerName = "";
	ContainerStaticInfo containerStaticInfo = {};
};

}