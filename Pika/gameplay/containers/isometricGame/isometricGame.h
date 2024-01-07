#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <fileChanged.h>
#include <engineLibraresSupport/engineGL2DSupport.h>
#include <containers/isometricGame/isometricGameEditor.h>


struct IsometricGame: public Container
{
	static constexpr int MAPS_COUNT = 5;

	IsometricGameEditor::Map levels[MAPS_COUNT] = {};

	gl2d::Renderer2D renderer;

	gl2d::Texture tiles;
	gl2d::Texture shadow;
	gl2d::TextureAtlasPadding tilesAtlas;

	gl2d::Texture playerSprite;
	gl2d::TextureAtlas playerAtlas;

	glm::ivec3 playerPosition = {7, 1, 7};

	struct PlayerAnimations
	{
		int indexX = 0;
		int indexY = 0;
		float timer = 0.2;
	}playerAnimations;

	std::vector<glm::ivec2> path;
	float timerPath = 0.2;


	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);


		info.requestImguiFbo = true;

		return info;
	}

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument);
	int currentBlock = 0;

	struct Block
	{
		unsigned char type;
		unsigned char secondType;

		void set(unsigned char count, unsigned char down)
		{
			type = count;
			secondType = down;
		}

		glm::ivec2 get()
		{
			return {type, secondType};
		}
	};

	IsometricGameEditor::Map map;

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo) override;

	void destruct(RequestedContainerInfo &requestedInfo) override;

};
