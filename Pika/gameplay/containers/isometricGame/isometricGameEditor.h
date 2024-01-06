#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <fileChanged.h>
#include <engineLibraresSupport/engineGL2DSupport.h>


struct IsometricGameEditor: public Container
{

	//todo some decorative piece and a hint
	//alimente care ii scresc viata

	enum Blocks
	{
		air = 0,
		clay,
		stone,
		ice,
		snowDirt,
		dirt,
		log,
		woddenPlank,
		redstone,
		trapdor,
		lever,
		redstoneTorch,
		redstoneBlock,
	};


	pika::FileChanged fileChanged;

	gl2d::Renderer2D renderer;
	pika::pikaImgui::FileSelector loadedLevel;

	gl2d::Texture tiles;
	gl2d::Texture shadow;
	gl2d::TextureAtlasPadding tilesAtlas;

	glm::ivec3 newMapSize = {};

	glm::ivec3 blockSelector = {};

	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		info.extensionsSuported = {".isomap"};

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

	struct Map
	{

		std::vector<Block> mapData;

		glm::ivec3 size = {};

		void init(glm::ivec3 size)
		{
			mapData.clear();
			mapData.resize(size.x * size.y * size.z);
			this->size = size;
		}

		void setSafe(glm::ivec3 pos, unsigned char count, bool down)
		{
			auto get = getSafe(pos);
			if (get) { get->set(count, down); }
		}

		void setSafe(glm::ivec3 pos, Block b)
		{
			auto get = getSafe(pos);
			if (get) { *get = b; }
		}

		Block *getSafe(glm::ivec3 pos)
		{
			if (pos.x >= 0 && pos.y >= 0 && pos.z >= 0
				&& pos.x < size.x && pos.y < size.y && pos.z < size.z
				)
			{
				return &mapData[pos.x * size.z * size.y + pos.y * size.z + pos.z];
			}
			else
			{
				return 0;
			}
		}

	};

	Map map;

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo) override;

	void destruct(RequestedContainerInfo &requestedInfo) override;

};
