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
	mario::NeuralSimulator simulator;
	//gl2d::FrameBuffer visionFBO;

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

	const int blockSizePreview = 50;

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		if (commandLineArgument.size() != 0)
		{
			mapFile = commandLineArgument.to_string();
		}

		bool rez = simulator.create(requestedInfo, mapFile);

		rez &= renderer.init(requestedInfo);

		//visionFBO.create(blockSizePreview * mario::NeuralSimulator::visionSizeX, blockSizePreview * mario::NeuralSimulator::visionSizeY);

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

		if (!simulator.updateFrame(input.deltaTime))
		{
			return false;
		}

		renderer.update(input, windowState, simulator.simulator);


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

			simulator.simulator.moveDelta = delta;

			if (input.buttons[pika::Button::Space].pressed())
			{
				simulator.simulator.jump = true;
			}
			else
			{
				simulator.simulator.jump = false;
			}

		}

		//visionFBO.clear();
		renderer.renderer.pushCamera();
		renderer.renderer.renderRectangle(
			glm::vec4(0, 0, mario::NeuralSimulator::visionSizeX * (float)blockSizePreview, mario::NeuralSimulator::visionSizeY * (float)blockSizePreview)
			, {0.5,0.5,0.5,0.5});
		for (int y = 0; y < mario::NeuralSimulator::visionSizeY; y++)
		{
			for (int x = 0; x < mario::NeuralSimulator::visionSizeX; x++)
			{
				auto b = simulator.vision[x + y * mario::NeuralSimulator::visionSizeX];
				if (b == 1)
				{
					renderer.renderer.renderRectangle(glm::vec4(x, y, 0.95, 0.95) * (float)blockSizePreview, {0,1,0,0.5});
				}
				//else
				//{
				//	renderer.renderer.renderRectangle(glm::vec4(x, y, 0.95, 0.95) * (float)blockSizePreview, {0.5,0.5,0.5,0.5});
				//}
			}
		}
		renderer.renderer.renderRectangle(
			glm::vec4(
			1 , 
			simulator.visionSizeY - 4,
			PLAYER_SIZE) *
			(float)blockSizePreview, {0,0,1,0.5});

		renderer.renderer.popCamera();
		//renderer.renderer.flushFBO(visionFBO);
		

		//renderer.renderer.renderRectangle({10,10, blockSizePreview * mario::NeuralSimulator::visionSizeX,blockSizePreview * mario::NeuralSimulator::visionSizeY},
		//	{}, 0, visionFBO.texture);



		glBindFramebuffer(GL_FRAMEBUFFER, requestedInfo.requestedFBO.fbo);
		renderer.render();

		//ImGui::Begin("Neural vision");
		//{
		//
		//
		//
		//}


		return true;
	}

	void destruct() override
	{
		renderer.cleanup();
		simulator.cleanup();
		//visionFBO.cleanup();
	}
};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release