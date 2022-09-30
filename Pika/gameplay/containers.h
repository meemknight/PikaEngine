#pragma once

#include <pikaConfig.h>
#include <baseContainer.h>
//this is used to declare containers




#include <memoryArena/memoryArena.h>


Container *getContainer(const char* name, pika::memory::MemoryArena *memoryArena);



#include "containers/pikaGameplay.h"
#include "pluggins/immageviewer.h"
