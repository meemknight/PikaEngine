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
	mario::Player player;
	
	int data[100];

	pika::FileChanged fileChanged;
	gl2d::FrameBuffer fbo;
	gl2d::ShaderProgram postProcessShader;
	pika::FileChanged vertexChanged;
	pika::FileChanged fragmentChanged;

	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		info.extensionsSuported = {".mario"};

		info.requestImguiFbo = true; //todo this should not affect the compatibility of input recording

		return info;
	}

	std::string mapFile = PIKA_RESOURCES_PATH "/mario/map1.mario";

	void loadShader(RequestedContainerInfo &requestedInfo)
	{

		auto vertData = requestedInfo.readEntireFileBinaryAsAString(PIKA_RESOURCES_PATH "mario/postProcess.vert");
		auto fragmeData = requestedInfo.readEntireFileBinaryAsAString(PIKA_RESOURCES_PATH "mario/postProcess.frag");

		postProcessShader = gl2d::createShaderProgram(vertData.c_str(), fragmeData.c_str());

	}

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		player.position.position = {1,1};
		player.lastPos = {1,1};

		if (commandLineArgument.size() != 0)
		{
			mapFile = commandLineArgument.to_string();
		}

		bool rez = simulator.create(requestedInfo, mapFile);

		rez &= renderer.init(requestedInfo);

		fileChanged.setFile(mapFile.c_str());

		fbo.create(1, 1);

		loadShader(requestedInfo);
		vertexChanged.setFile(PIKA_RESOURCES_PATH "mario/postProcess.vert");
		fragmentChanged.setFile(PIKA_RESOURCES_PATH "mario/postProcess.frag");

		return rez;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
		fbo.clear();
		fbo.resize(windowState.frameBufferW, windowState.frameBufferH);
		//glBindFramebuffer(GL_FRAMEBUFFER, requestedInfo.requestedFBO.fbo);
		renderer.renderer.clearScreen();

		if (fileChanged.changed())
		{
			mario::loadMap(requestedInfo, mapFile, &simulator.map, simulator.mapSize);
		}

		if (vertexChanged.changed() || fragmentChanged.changed())
		{
			glDeleteShader(postProcessShader.id);
			loadShader(requestedInfo);
		}
		//requestedInfo.readEntireFile


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

			if (!simulator.updateFrame(input.deltaTime, player))
			{
				player.position.position = {1,1};
				player.lastPos = {1,1};
			}
		
		}
		
		renderer.update(input, windowState, simulator);
		renderer.followPlayer(player, input, windowState);
		renderer.drawPlayer(player);
		
		
		renderer.renderer.flushFBO(fbo);
		renderer.renderer.pushCamera();
		renderer.renderer.pushShader(postProcessShader);
		renderer.renderer.renderRectangle({0,0, windowState.frameBufferW, windowState.frameBufferH}, fbo.texture);
		renderer.renderer.flush();
		renderer.renderer.popShader();
		renderer.renderer.popCamera();

		renderer.render();


		return true;
	}

	void destruct(RequestedContainerInfo &requestedInfo) override
	{
		renderer.cleanup();
		simulator.cleanup();
		fbo.cleanup();
		glDeleteShader(postProcessShader.id);
	}

};
