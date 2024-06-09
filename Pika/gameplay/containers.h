#pragma once

#include <pikaConfig.h>
#include <baseContainer.h>
//this is used to declare containers




#include <memoryArena/memoryArena.h>


Container *getContainer(const char* name, pika::memory::MemoryArena *memoryArena);


#include "containers/pikaGameplay.h"
#include "containers/threedtest.h"

#include "pluggins/immageviewer.h"
#include "pluggins/threeDEditor.h"
#include "pluggins/pikatextEditor.h"
#include "pluggins/sushiViewer/sushiViewer.h"

#include "containers/mario/mario.h"
#include "containers/mario/marioEditor.h"
#include "containers/mario/marioNeuralVizualizer.h"
#include "containers/mario/marioNeuralTrainer.h"
#include "containers/minecraftDungeons/mcDungeonsEditor.h"
#include "containers/minecraftDungeons/mcDungeonsgameplay.h"
#include "containers/minecraftDungeons/mcDungeonsMenu.h"
#include "containers/hollowknight/hollowknight.h"

#include "containers/marioKart/marioKartEditor.h"
#include "containers/marioKart/marioKart.h"

#include "containers/threeDGameExample/threeDGameExample.h"
#include "containers/threeDGameExample/threeDGameMenu.h"


#include "containers/isometricGame/isometricGameEditor.h"
#include "containers/isometricGame/isometricGame.h"

#include "containers/bezie/bezie.h"


#if PIKA_PRODUCTION == 1

#define PIKA_ALL_CONTAINERS() \
	//PIKA_DECLARE_CONTAINER(McDungeonsGameplay) \
	//PIKA_DECLARE_CONTAINER(McDungeonsMenu) 
	//PIKA_DECLARE_CONTAINER(Gameplay) 

#else

#define PIKA_ALL_CONTAINERS() \
	PIKA_DECLARE_CONTAINER(Gameplay) \
	PIKA_DECLARE_CONTAINER(SushiViewer) \
	PIKA_DECLARE_CONTAINER(ImmageViewer) \
	PIKA_DECLARE_CONTAINER(ThreeDTest) \
	PIKA_DECLARE_CONTAINER(ThreeDEditor) \
	PIKA_DECLARE_CONTAINER(PikaTextEditor) \
	PIKA_DECLARE_CONTAINER(ThreeDGameExample) \
	PIKA_DECLARE_CONTAINER(ThreeDGameMenu) \
	PIKA_DECLARE_CONTAINER(MarioKartEditor) \
	PIKA_DECLARE_CONTAINER(MarioKart) \
	PIKA_DECLARE_CONTAINER(IsometricGameEditor) \
	PIKA_DECLARE_CONTAINER(Mario) \
	PIKA_DECLARE_CONTAINER(Holloknight)\
	PIKA_DECLARE_CONTAINER(Bezie)\
	PIKA_DECLARE_CONTAINER(IsometricGame) \
	PIKA_DECLARE_CONTAINER(MarioEditor) \
	PIKA_DECLARE_CONTAINER(MarioNeuralTrainer) \
	PIKA_DECLARE_CONTAINER(MarioNeuralVizualizer) 
	//PIKA_DECLARE_CONTAINER(McDungeonsGameplay) \
	//PIKA_DECLARE_CONTAINER(McDungeonsEditor)
	//PIKA_DECLARE_CONTAINER(McDungeonsMenu)

	

#endif