#pragma once
#include <gl2d/gl2d.h>
#include <baseContainer.h>

namespace mario
{


constexpr const char *collisionMap =
"XX-X-XXX"
"XX-XXXXX"
"--------"
"-XX--X--"
"-XX-----"
"-XX-XXXX"
"--X-XXX-"
"XX------"
"XX--XX--"
"XXXXXX--"
;

bool isSolid(int id);

glm::vec4 getTileUV(gl2d::TextureAtlasPadding atlas, int id, int flip = 0);

struct Block
{
	unsigned char type : 7;
	unsigned char flipped : 1;

	bool isCollidable()
	{
		return collisionMap[type] == 'X';
	}
};



bool aabb(glm::vec4 b1, glm::vec4 b2, float delta);

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

bool loadMap(RequestedContainerInfo &requestedInfo, std::string file, Block **map, glm::ivec2 &mapSize);

struct GameplaySimulation
{

	GameplaySimulation() {}

	bool create(RequestedContainerInfo &requestedInfo, std::string file)
	{
		player.position.position = {1,1};

		bool rez = loadMap(requestedInfo, file, &map, mapSize);

		return rez;
	}

	Player player;

	Block *map;
	glm::ivec2 mapSize = {100, 100};

	Block &getMapBlockUnsafe(int x, int y)
	{
		return map[x + y * mapSize.x];
	}


	int moveDelta = 0;
	bool jump = 0;

	
	void updateFrame(float deltaTime)
	{
		player.input = moveDelta;
		if(jump)player.jump(50);

		//phisics
		{
			player.moveVelocityX(5 * deltaTime * player.input);

			player.applyGravity(200.f * deltaTime);

			player.updatePhisics(deltaTime);

			player.grounded = false;
			player.resolveConstrains(map);

			//player.playerAnimation.grounded = i.second.grounded;

			player.updateMove();

			//player.playerAnimation.update(input.deltaTime);
		}


	}


};

struct GameplayRenderer
{
	gl2d::Renderer2D renderer;
	gl2d::Texture tiles;
	gl2d::Texture marioTexture;
	gl2d::TextureAtlasPadding atlas;

	bool init(RequestedContainerInfo &requestedInfo)
	{
		renderer.create();
		//gl2d::setErrorFuncCallback() //tood
		//pika::initShortcutApi();

		size_t s = 0;
		if (requestedInfo.getFileSizeBinary(PIKA_RESOURCES_PATH "/mario/1985_tiles.png", s))
		{
			void *data = new unsigned char[s];
			if (requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "/mario/1985_tiles.png", data, s))
			{
				tiles.createFromFileDataWithPixelPadding((unsigned char *)data, s, 8, true, false);

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


		atlas = gl2d::TextureAtlasPadding(8, 10, 8 * 8, 8 * 10);

		renderer.currentCamera.zoom = 60.f;
	
		return 1;
	}


	void update(pika::Input input, pika::WindowState windowState,
		GameplaySimulation &simulator)
	{
		{
			glClear(GL_COLOR_BUFFER_BIT);
			gl2d::enableNecessaryGLFeatures();
			renderer.updateWindowMetrics(windowState.w, windowState.h);
		}

		float wheel = ImGui::GetIO().MouseWheel;
		//todo standard out

		if ((ImGui::GetIO().KeysData[ImGuiKey_LeftCtrl].Down || ImGui::GetIO().KeysData[ImGuiKey_RightCtrl].Down) && input.hasFocus)
		{
			renderer.currentCamera.zoom += wheel * 3;
		}

		renderer.currentCamera.zoom = std::min(renderer.currentCamera.zoom, 70.f);
		renderer.currentCamera.zoom = std::max(renderer.currentCamera.zoom, 50.f);

		//todo update gl2d this function
		renderer.currentCamera.follow(simulator.player.position.getCenter(), input.deltaTime * 3, 0.0001, 0.2, windowState.w, windowState.h);
		

		auto viewRect = renderer.getViewRect();

		glm::ivec2 minV;
		glm::ivec2 maxV;
		//render
		{

			minV = {viewRect.x - 2, viewRect.y - 2};
			maxV = minV + glm::ivec2{viewRect.z + 4, viewRect.w + 4};
			minV = glm::max(minV, {0,0});
			maxV = glm::min(maxV, simulator.mapSize);


			for (int j = minV.y; j < maxV.y; j++)
				for (int i = minV.x; i < maxV.x; i++)
				{
					auto b = simulator.getMapBlockUnsafe(i, j);
					auto uv = getTileUV(atlas, b.type, b.flipped);

					renderer.renderRectangle({i, j, 1, 1}, {}, {}, tiles, uv);

				}
		}

		glm::vec4 pos(simulator.player.position.position, 1, 1);
		//pos.y -= 1 / 8.f;
		pos.x -= 1 / 8.f;

		renderer.renderRectangle(pos, {}, {}, marioTexture,
			simulator.player.movingRight ? glm::vec4(0, 1, 1, 0) : glm::vec4(1, 1, 0, 0));


		renderer.flush();


	}



};

};
