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
	pika::pikaImgui::FileSelector rezFile;
	pika::pikaImgui::FileSelector graphFile;

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(400);

		info.extensionsSuported = {".mario"};

		info.requestImguiFbo = true;
		info.pushAnImguiIdForMe = true;

		return info;
	}
	
	struct SimulationNetwork
	{
		bool mutation = 0;
		mario::PlayerSimulation player;
		mario::NeuralNetork network;
	};

	std::vector<SimulationNetwork> simulations;
	std::vector<SimulationNetwork> deadSimulations;

	std::mt19937 rng = std::mt19937(std::random_device{}());

	int generation = 1;

	bool preview = 1;
	bool fast = 0;
	bool save = 0;

	void recreateGenerations(RequestedContainerInfo &requestedInfo)
	{
		simulations.clear();
		std::sort(deadSimulations.begin(), deadSimulations.end(), [&](SimulationNetwork &a, SimulationNetwork &b) 
		{
			return 	a.player.maxFit > b.player.maxFit;
		});


		if (rezFile.file[0] != '\0' && save)
		{
			requestedInfo.writeEntireFileBinary(rezFile.file, &deadSimulations[0].network, sizeof(deadSimulations[0].network));
		}

		if (graphFile.file[0] != '\0' && save)
		{

			::pika::memory::pushCustomAllocatorsToStandard();
			{
				std::ofstream file(graphFile.file);

				if (file.is_open())
				{
					for (auto &g : generations)
					{
						file << g.leastFit << " " << g.averageFit << " " << g.fitest << "\n";
					}
				};

				file.close();
			}
			::pika::memory::popCustomAllocatorsToStandard();

		}

		//we keep the first people unchanged
		std::vector<SimulationNetwork> fittest;
		fittest.push_back(deadSimulations[0]);
		fittest.push_back(deadSimulations[1]);
		fittest.push_back(deadSimulations[2]);
		fittest.push_back(deadSimulations[3]);
		fittest.push_back(deadSimulations[4]);

		for (auto &i : fittest) 
		{ 
			i.player = {};
			i.player.p.position.position = {1,1};
			i.player.p.lastPos = {1,1};
			i.mutation = 0;
			simulations.push_back(i); 
		}

		//we also add some of the less fit people
		for (int i = 0; i < 10; i++)
		{
			float a = mario::getRandomFloat(rng, 0, 1);
			float b = mario::getRandomFloat(rng, 0, 1);

			a *= b; //we bias the people to keep the fitter ones

			fittest.push_back(deadSimulations[5 + a * (deadSimulations.size()/2)]);
		}

		int size = fittest.size() - 1;

		while (simulations.size() < 10000)
		{
			int a = mario::getRandomInt(rng, 0, size);
			int b = mario::getRandomInt(rng, 0, size);

			SimulationNetwork newNetwork = fittest[a];
			newNetwork.network.combine(rng, fittest[b].network);
			
			newNetwork.player = {};
			newNetwork.player.p.position.position = {1,1};
			newNetwork.player.p.lastPos = {1,1};

			//combine the 2 individuals
			
			if (mario::getRandomChance(rng, 0.25))
			{
				newNetwork.mutation = 1;

				int type = mario::getRandomInt(rng, 0, 120);

				if (type < 10)
				{
					newNetwork.network.removeRandomWeight(rng);
					newNetwork.network.removeRandomWeight(rng);
					newNetwork.network.removeRandomWeight(rng);
				}
				if (type < 25)
				{
					newNetwork.network.removeRandomWeight(rng);
				}
				else if (type < 50)
				{
					newNetwork.network.addRandomConnection(rng);
				}
				else if (type < 75)
				{
					newNetwork.network.addRandomConnection(rng);
					newNetwork.network.addRandomConnection(rng);
				}
				else if (type < 100)
				{
					newNetwork.network.changeRandomWeight(rng);
				}
				else if (type <= 120)
				{
					newNetwork.network.changeRandomWeight(rng);
					newNetwork.network.changeRandomWeight(rng);
					newNetwork.network.changeRandomWeight(rng);
				}

			}
			simulations.push_back(newNetwork);
		}

		for (auto &s : simulations)
		{
			s.player.p.position.position.y += mario::getRandomFloat(rng, 0, 5);
		}

		generation++;
		deadSimulations.clear();
	}

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		currentMap.setInfo("Training map", PIKA_RESOURCES_PATH "/mario", {".mario"});

		rezFile.setInfo("Neural network file", PIKA_RESOURCES_PATH "/mario", {".neural"});
		graphFile.setInfo("Graph File", PIKA_RESOURCES_PATH "/mario", {".txt"});


		if (commandLineArgument.size() != 0)
		{
			pika::strlcpy(currentMap.file, commandLineArgument.to_string(), sizeof(currentMap.file));
		}
		else
		{
			pika::strlcpy(currentMap.file, PIKA_RESOURCES_PATH "/mario/map1.mario", sizeof(currentMap.file));
		}

		bool rez = simulator.create(requestedInfo, currentMap.file);

		rez &= renderer.init(requestedInfo);

		std::mt19937 rng(std::random_device{}());

		simulations.reserve(100);
		for (int i = 0; i < 100; i++)
		{
			SimulationNetwork s;
			s.network.addRandomConnection(rng);
			s.network.addRandomConnection(rng);
			s.network.addRandomConnection(rng);
			s.network.addRandomConnection(rng);
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

	struct Generation
	{
		float fitest = 0;
		float leastFit = 0;
		float averageFit = 0;
	};

	std::vector<Generation> generations;

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{

		//simulator.moveDelta = 0;
		//simulator.jump = false;

		int maxIndex = 0;
		float maxFit = 0;
		for (int i = 0; i < simulations.size(); i++)
		{
			//if (simulations[i].player.p.position.position.x - simulations[i].player.jumpCount / 10.f > maxFit)
			//{
			//	maxFit = simulations[i].player.p.position.position.x - simulations[i].player.jumpCount/10.f;
			//	maxIndex = i;
			//}

			if (simulations[i].player.maxFit > maxFit)
			{
				maxFit = simulations[i].player.maxFit;
				maxIndex = i;
			}
		}

		if (preview)
		{
			renderer.update(input, windowState, simulator);

			for (int i = 0; i < simulations.size(); i++)
			renderer.drawPlayer(simulations[i].player.p);

			renderer.followPlayer(simulations[maxIndex].player.p, input, windowState);

			{
				char vision[mario::visionSizeX * mario::visionSizeY] = {};
				getVision(vision, simulator, simulations[maxIndex].player);
				mario::renderNeuralNetwork(renderer.renderer, vision, 20, simulations[maxIndex].network);
			}
		}



		float newDelta = input.deltaTime;

		if (fast)
		{
			newDelta = 1.f / 20.f;
		}

		for (int i = 0; i < simulations.size(); i++)
		{

			if (!mario::performNeuralSimulation(simulations[i].player, 
				newDelta, simulator, simulations[i].network))
			{
				deadSimulations.push_back(simulations[i]);

				if (simulations.size() == 1)
				{
					simulations.clear();
				}
				else
				{
					simulations[i] = simulations[simulations.size() - 1];
					simulations.pop_back();
				}
			
				i--;
				continue;
			}


		}

		if (simulations.empty())
		{

			Generation result = {};

			result.fitest = 0;
			result.leastFit = 9999;
			result.averageFit = 0;

			for (auto &g : deadSimulations)
			{
				if (g.player.maxFit > result.fitest)
				{
					result.fitest = g.player.maxFit;
				}

				if (g.player.maxFit < result.leastFit && !g.mutation)
				{
					result.leastFit = g.player.maxFit;
				}

				result.averageFit += g.player.maxFit;
			}

			result.averageFit /= deadSimulations.size();

			if (result.leastFit == 9999)
			{
				result.leastFit = 1;
			}

			if (result.leastFit >= result.averageFit)
			{
				result.leastFit = 1;
			}

			generations.push_back(result);

			recreateGenerations(requestedInfo);
		}

		
		if (preview)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, requestedInfo.requestedFBO.fbo);
			renderer.render();
		}
		else
		{
			renderer.renderer.clearDrawData();
		}



		ImGui::Begin("Neural trainer");
		{
			ImGui::Text("Trainer");
			ImGui::Separator();
			currentMap.run(1);
			ImGui::Separator();
			ImGui::Text("Generation: %d", generation);
			ImGui::Text("Current individuals alieve: %d", (int)simulations.size());
			ImGui::Text("Float fittest dist: %f", maxFit);
			ImGui::Checkbox("preview", &preview);
			ImGui::Checkbox("fixed framerate", &fast);

			rezFile.run(2);
			graphFile.run(3);
			//ImGui::SameLine();
			ImGui::Checkbox("save", &save);

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