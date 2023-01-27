#pragma once

#include <pikaConfig.h>
#include <baseContainer.h>
//this is used to declare containers




#include <memoryArena/memoryArena.h>


Container *getContainer(const char* name, pika::memory::MemoryArena *memoryArena);


#include "containers/pikaGameplay.h"
#include "containers/threedtest.h"
#include "containers/mario/mario.h"
#include "containers/mario/marioEditor.h"
#include "pluggins/immageviewer.h"
#include "pluggins/threeDEditor.h"
#include "pluggins/pikatextEditor.h"

#define PIKA_ALL_CONTAINERS() \
	PIKA_DECLARE_CONTAINER(Gameplay) \
	PIKA_DECLARE_CONTAINER(ImmageViewer) \
	PIKA_DECLARE_CONTAINER(ThreeDTest) \
	PIKA_DECLARE_CONTAINER(ThreeDEditor) \
	PIKA_DECLARE_CONTAINER(PikaTextEditor) \
	PIKA_DECLARE_CONTAINER(Mario) \
	PIKA_DECLARE_CONTAINER(MarioEditor)
