#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include "marioCommon.h"
#include <fileChanged.h>

struct Transform
{
	glm::vec2 position = {};
	glm::vec2 size = {};

	glm::vec2 getTopLeftCorner();
	glm::vec2 getCenter();
	glm::vec2 getBottomLeftCorner();
	glm::vec2 getBottomCenter();

};

#define PLAYER_SIZE glm::vec2(6.f / 8.f, 1.f)

struct Player
{
	Transform position = {glm::vec2(0,0), PLAYER_SIZE};

	glm::vec2 lastPos{};

	glm::vec2 velocity = {};

	bool movingRight = 0;
	bool grounded = 0;

	void move(glm::vec2 dir);

	void moveVelocityX(float dir);

	void jump(float power);

	void applyGravity(float gravity);

	bool movingThisFrame = false;


	//should be called only once per frame last
	void updateMove();

	//this should be called before collisions 
	void updatePhisics(float deltaTime);


	void resolveConstrains(Block *map);

	void checkCollisionBrute(glm::vec2 &pos, glm::vec2 lastPos,
		Block *map, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch);

	glm::vec2 Player::performCollision(Block *map, glm::vec2 pos, glm::vec2 size,
		glm::vec2 delta, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch);

	int input = 0;
};

struct Mario: public Container
{

	gl2d::Renderer2D renderer;
	gl2d::Texture tiles;
	gl2d::Texture marioTexture;
	gl2d::TextureAtlasPadding atlas;
	
	Player player;
	pika::FileChanged fileChanged;

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		info.requestImguiFbo = true; //todo this should not affect the compatibility of input recording


		return info;
	}

	Block *map;
	glm::ivec2 mapSize = {100, 100};

	Block &getMapBlockUnsafe(int x, int y)
	{
		return map[x + y * mapSize.x];
	}

	bool loadMap(RequestedContainerInfo &requestedInfo)
	{
		size_t s = 0;
		if (requestedInfo.getFileSizeBinary(PIKA_RESOURCES_PATH "/mario/map1.mario", s))
		{
			if (s == mapSize.x * mapSize.y)
			{
				requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "/mario/map1.mario", map, mapSize.x * mapSize.y);
			}
			else { return 0; }
		}
		else { return 0; }

		return 1;
	}

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		player.position.position = {1,1};

		renderer.create();
		//gl2d::setErrorFuncCallback() //tood
		//pika::initShortcutApi();

		size_t s = 0;
		if (requestedInfo.getFileSizeBinary(PIKA_RESOURCES_PATH "/mario/1985_tiles.png", s))
		{
			void *data = new unsigned char[s];
			if (requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "/mario/1985_tiles.png", data, s))
			{
				tiles.createFromFileDataWithPixelPadding((unsigned char*)data, s, 8, true, false);

			}
			else { return 0; }

			delete[] data;
		}
		else { return 0; }

		//todo push pop or sthing
		pika::memory::setGlobalAllocatorToStandard();
		marioTexture.loadFromFile(PIKA_RESOURCES_PATH "/mario/mario.png", true, false);
		marioTexture.loadFromFile(PIKA_RESOURCES_PATH "/mario/mario.png", true, false);
		pika::memory::setGlobalAllocator(requestedInfo.mainAllocator);


		atlas = gl2d::TextureAtlasPadding(8, 10, 8*8, 8*10);

		map = new Block[mapSize.x * mapSize.y];
		Block d{27,0};
		memset(map, *(int *)(&d), mapSize.x * mapSize.y);

		renderer.currentCamera.zoom = 60.f;
		
		bool rez = loadMap(requestedInfo);

		fileChanged.setFile(PIKA_RESOURCES_PATH "/mario/map1.mario");

		return rez;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{

		{
			glClear(GL_COLOR_BUFFER_BIT);
			gl2d::enableNecessaryGLFeatures();
			renderer.updateWindowMetrics(windowState.w, windowState.h);
		}

		if (fileChanged.changed())
		{
			loadMap(requestedInfo);
		}

		{
			float wheel = ImGui::GetIO().MouseWheel;

			//todo standard out

			if ((ImGui::GetIO().KeysData[ImGuiKey_LeftCtrl].Down || ImGui::GetIO().KeysData[ImGuiKey_RightCtrl].Down) && input.hasFocus)
			{
				renderer.currentCamera.zoom += wheel * 3;
			}

			renderer.currentCamera.zoom = std::min(renderer.currentCamera.zoom, 70.f);
			renderer.currentCamera.zoom = std::max(renderer.currentCamera.zoom, 50.f);

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

			player.input = delta;

			if (input.buttons[pika::Button::Space].pressed() && player.grounded)
			{
				player.jump(60);
			}

			//phisics
			{
				player.moveVelocityX(5 * input.deltaTime * player.input);
				
				player.applyGravity(300.f * input.deltaTime);

				player.updatePhisics(input.deltaTime);

				player.grounded = false;
				player.resolveConstrains(map);

				//player.playerAnimation.grounded = i.second.grounded;

				player.updateMove();

				//player.playerAnimation.update(input.deltaTime);
			}

			//todo update gl2d this function
			renderer.currentCamera.follow(player.position.getCenter(), input.deltaTime * 3, 0.0001, 0.2, windowState.w, windowState.h);

		}
		auto viewRect = renderer.getViewRect();

		glm::ivec2 minV;
		glm::ivec2 maxV;
		//render
		{

			minV = {viewRect.x - 2, viewRect.y - 2};
			maxV = minV + glm::ivec2{viewRect.z + 4, viewRect.w + 4};
			minV = glm::max(minV, {0,0});
			maxV = glm::min(maxV, mapSize);


			for (int j = minV.y; j < maxV.y; j++)
				for (int i = minV.x; i < maxV.x; i++)
				{
					auto b = getMapBlockUnsafe(i, j);
					auto uv = getTileUV(atlas, b.type, b.flipped);

					renderer.renderRectangle({i, j, 1, 1}, {}, {}, tiles, uv);

				}
		}

		glm::vec4 pos(player.position.position, 1, 1);
		//pos.y -= 1 / 8.f;
		pos.x -= 1 / 8.f;

		renderer.renderRectangle(pos, {}, {}, marioTexture, 
			player.movingRight ? glm::vec4(0,1,1,0) : glm::vec4(1, 1, 0, 0));


		renderer.flush();


		return true;
	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release