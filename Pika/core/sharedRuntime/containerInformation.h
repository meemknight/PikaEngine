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

	bool operator==(const ContainerInformation &other)
	{
		if (this == &other) { return true; }

		return
			this->containerStructBaseSize == other.containerStructBaseSize &&
			this->containerName == other.containerName &&
			this->containerStaticInfo == other.containerStaticInfo;

	}

	bool operator!=(const ContainerInformation &other)
	{ 
		return !(*this == other);
	}

	size_t containerStructBaseSize = 0; //static memory
	std::string containerName = "";
	ContainerStaticInfo containerStaticInfo = {};
};

}