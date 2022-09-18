#pragma once

#include <pikaConfig.h>
#include <baseContainer.h>
//this is used to declare containers




#include "containers/pikaGameplay.h"
#include <memoryArena/memoryArena.h>


Container *getContainer(const char* name, pika::memory::MemoryArena *memoryArena);