#pragma once

#include <gl3d.h>
#include <baseContainer.h>


void errorCallbackCustom(std::string err, void *userData);
std::string readEntireFileCustom(const char *fileName, bool &couldNotOpen, void *userData);
std::vector<char> readEntireFileBinaryCustom(const char *fileName, bool &couldNotOpen, void *userData);
bool defaultFileExistsCustom(const char *fileName, void *userData);
