#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include "marioCommon.h"
#include <fileChanged.h>



struct Mario: public Container
{

	mario::GameplaySimulation simulator;
	mario::GameplayRenderer renderer;
	
	pika::FileChanged fileChanged;

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		info.extensionsSuported = {".mario"};

		info.requestImguiFbo = true; //todo this should not affect the compatibility of input recording

		return info;
	}

	std::string mapFile = PIKA_RESOURCES_PATH "/mario/map1.mario";

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		if (commandLineArgument.size() != 0)
		{
			mapFile = commandLineArgument.to_string();
		}

		bool rez = simulator.create(requestedInfo, mapFile);

		rez &= renderer.init(requestedInfo);

		fileChanged.setFile(mapFile.c_str());

		return rez;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{

		if (fileChanged.changed())
		{
			mario::loadMap(requestedInfo, mapFile, &simulator.map, simulator.mapSize);
		}

		{
			int delta = 0;

			if (input.hasFocus)
			{
				if (input.buttons[pika::Button::A].held())
				{
					delta -= 1;
				}
				if (input.buttons[pika::Button::D].held())
				{
					delta += 1;
				}
				
			}

			simulator.moveDelta = delta;

			if (input.buttons[pika::Button::Space].pressed())
			{
				simulator.jump = true;
			}
			else
			{
				simulator.jump = false;
			}

			simulator.updateFrame(input.deltaTime);

		
		}
		
		renderer.update(input, windowState, simulator);

		return true;
	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release