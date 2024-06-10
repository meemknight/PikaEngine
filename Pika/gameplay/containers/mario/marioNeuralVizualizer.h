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

	pika::pikaImgui::FileSelector rezFile;

	bool fixedFramerate = 0;

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
		rezFile.setInfo("Neural network file", PIKA_RESOURCES_PATH "/mario", {".neural"});

		if (commandLineArgument.size() != 0)
		{
			mapFile = commandLineArgument.to_string();
		}

		bool rez = simulator.create(requestedInfo, mapFile);

		rez &= renderer.init(requestedInfo);

		std::mt19937 rng(std::random_device{}());

		network.addRandomConnection(rng);
		network.addRandomConnection(rng);
		network.addRandomConnection(rng);
		network.addRandomConnection(rng);

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

		float deltaTime = input.deltaTime;

		if (fixedFramerate)
		{
			deltaTime = 1.f / 50.f;
		}

		if (!mario::performNeuralSimulation(player, deltaTime, simulator, network))
		{
			player = {};
			player.p.position.position = {1,1};
			player.p.lastPos = {1,1};
		}

		char vision[mario::visionSizeX * mario::visionSizeY] = {};
		mario::getVision(vision, simulator, player);
		mario::renderNeuralNetwork(renderer.renderer, vision, 20, network);
		


		glBindFramebuffer(GL_FRAMEBUFFER, requestedInfo.requestedFBO.fbo);
		renderer.render();

		ImGui::Begin("Neural trainer for visualizer");
		{
		
			if (rezFile.run(2))
			{
				requestedInfo.readEntireFileBinary(rezFile.file, &network, sizeof(network));
			}

			ImGui::Checkbox("Fixed framerate", &fixedFramerate);

			ImGui::Text("Pos: %d", (int)player.p.position.position.x);

		}
		ImGui::End();


		return true;
	}

	void destruct(RequestedContainerInfo &requestedInfo) override
	{
		renderer.cleanup();
		simulator.cleanup();
	}
};
