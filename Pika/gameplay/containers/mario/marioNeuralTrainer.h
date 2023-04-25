#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include "marioCommon.h"
#include <fileChanged.h>




struct MarioNeuralTrainer: public Container
{

	mario::GameplayRenderer renderer;
	mario::GameplaySimulation simulator;
	pika::pikaImgui::FileSelector currentMap;

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(100);

		info.extensionsSuported = {".mario"};

		info.requestImguiFbo = true;
		info.pushAnImguiIdForMe = true;

		return info;
	}
	
	struct SimulationNetwork
	{
		mario::PlayerSimulation player;
		mario::NeuralNetork network;
	};

	std::vector<SimulationNetwork> simulations;

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		currentMap.setInfo("Training map", PIKA_RESOURCES_PATH "/mario", {".mario"});

		if (commandLineArgument.size() != 0)
		{
			pika::strlcpy(currentMap.file, commandLineArgument.data(), sizeof(currentMap.file));
		}
		else
		{
			pika::strlcpy(currentMap.file, PIKA_RESOURCES_PATH "/mario/map1.mario", sizeof(currentMap.file));
		}

		bool rez = simulator.create(requestedInfo, currentMap.file);

		rez &= renderer.init(requestedInfo);

		std::mt19937 rng(std::random_device{}());

		simulations.reserve(200);
		for (int i = 0; i < 200; i++)
		{
			SimulationNetwork s;
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.network.addRandomNeuron(rng);
			s.player.p.position.position = {1,1};
			s.player.p.lastPos = {1,1};
			simulations.push_back(s);
		}

		return rez;
	}

	float getFract(float v)
	{
		return v - (int)v;
	}


	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{

		//simulator.moveDelta = 0;
		//simulator.jump = false;

		int maxIndex = 0;
		float maxFit = 0;
		for (int i = 0; i < simulations.size(); i++)
		{
			if (simulations[i].player.p.position.position.x > maxFit)
			{
				maxFit = simulations[i].player.p.position.position.x;
				maxIndex = i;
			}
		}

		renderer.update(input, windowState, simulator);
		renderer.followPlayer(simulations[maxIndex].player.p, input, windowState);

		for (int i = 0; i < simulations.size(); i++)
		renderer.drawPlayer(simulations[i].player.p);

		{
			char vision[mario::visionSizeX * mario::visionSizeY] = {};
			getVision(vision, simulator, simulations[maxIndex].player);
			mario::renderNeuralNetwork(renderer.renderer, vision, 20, simulations[maxIndex].network);
		}

		for (int i = 0; i < simulations.size(); i++)
		{

			if (!mario::performNeuralSimulation(simulations[i].player, 
				input.deltaTime, simulator, simulations[i].network))
			{
				simulations.erase(simulations.begin() + i);
				i--;
				continue;
			}


		}

		if (simulations.empty()) { return 0; }

		
		//simulator.simulator.player.position.position.x;



		glBindFramebuffer(GL_FRAMEBUFFER, requestedInfo.requestedFBO.fbo);
		renderer.render();

		ImGui::Begin("Neural trainer");
		{
			ImGui::Text("Trainer");
			ImGui::Separator();
			currentMap.run(1);
			ImGui::Separator();



		}
		ImGui::End();

		return true;
	}

	void destruct() override
	{
		renderer.cleanup();
		simulator.cleanup();
	}



};