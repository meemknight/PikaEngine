#pragma once
#include <string>

struct ContainerInformation
{
	ContainerInformation() {};
	ContainerInformation(
	size_t containerStructBaseSize,
	const char *containerName):containerStructBaseSize(containerStructBaseSize), containerName(containerName) {};

	size_t containerStructBaseSize = 0;
	std::string containerName = "";

};