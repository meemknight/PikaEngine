#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include "marioCommon.h"
#include <fileChanged.h>



struct MarioNeuralVizualizer: public Container
{

	mario::GameplayRenderer renderer;
	mario::GameplaySimulation simulator;
	mario::PlayerSimulation player;

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		info.extensionsSuported = {".mario"};

		info.requestImguiFbo = true;
		info.pushAnImguiIdForMe = true;

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

		std::mt19937 rng(std::random_device{}());

		network.addRandomNeuron(rng);
		network.addRandomNeuron(rng);
		network.addRandomNeuron(rng);

		player.p.position.position = {1,1};
		player.p.lastPos = {1,1};

		return rez;
	}

	float getFract(float v)
	{
		return v - (int)v;
	}

	mario::NeuralNetork network;

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{

		//simulator.moveDelta = 0;
		//simulator.jump = false;

		renderer.update(input, windowState, simulator);
		renderer.followPlayer(player.p, input, windowState);
		renderer.drawPlayer(player.p);

		if (!mario::performNeuralSimulation(player, input.deltaTime, simulator, network))
		{
			return 0;
		}

		char vision[mario::visionSizeX * mario::visionSizeY] = {};
		mario::getVision(vision, simulator, player);
		mario::renderNeuralNetwork(renderer.renderer, vision, 20, network);
		
		//simulator.simulator.player.position.position.x;
	


		glBindFramebuffer(GL_FRAMEBUFFER, requestedInfo.requestedFBO.fbo);
		renderer.render();


		return true;
	}

	void destruct() override
	{
		renderer.cleanup();
		simulator.cleanup();
	}
};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release