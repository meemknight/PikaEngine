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
"-XX-XXX-"
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


	void resolveConstrains(Block *map, glm::ivec2 mapSize);

	void checkCollisionBrute(glm::vec2 &pos, glm::vec2 lastPos,
		Block *map, glm::ivec2 mapSize, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch);

	glm::vec2 Player::performCollision(Block *map, glm::ivec2 mapSize, glm::vec2 pos, glm::vec2 size,
		glm::vec2 delta, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch);

	int input = 0;
};

bool loadMap(RequestedContainerInfo &requestedInfo, std::string file, Block **map, glm::ivec2 &mapSize);

struct GameplaySimulation
{

	GameplaySimulation() {}

	bool create(RequestedContainerInfo &requestedInfo, std::string file)
	{
		//player.position.position = {1,1};

		bool rez = loadMap(requestedInfo, file, &map, mapSize);

		return rez;
	}

	Block *map;
	glm::ivec2 mapSize = {100, 100};

	Block &getMapBlockUnsafe(int x, int y)
	{
		return map[x + y * mapSize.x];
	}

	Block getMapBlockSafe(int x, int y)
	{
		if (x < 0 || y < 0 || x >= mapSize.x || y >= mapSize.y)
		{
			return {27};
		}

		return map[x + y * mapSize.x];
	}

	int moveDelta = 0;
	bool jump = 0;

	bool updateFrame(float deltaTime, Player &player)
	{
		player.input = moveDelta;
		if(jump)player.jump(50);

		//phisics
		{
			player.move({18 * deltaTime * player.input, 0});

			player.applyGravity(200.f * deltaTime);

			player.updatePhisics(deltaTime);

			player.grounded = false;
			player.resolveConstrains(map, mapSize);

			//player.playerAnimation.grounded = i.second.grounded;

			player.updateMove();

			//player.playerAnimation.update(input.deltaTime);
		}

		//check fall out
		if (player.position.getTopLeftCorner().y > mapSize.y)
		{
			return 0;
		}

		return true;
	}

	void cleanup()
	{
		delete[] map;
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
		renderer.create(requestedInfo.requestedFBO.fbo);
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

		pika::memory::pushCustomAllocatorsToStandard();
		marioTexture.loadFromFile(PIKA_RESOURCES_PATH "/mario/mario.png", true, false);
		pika::memory::popCustomAllocatorsToStandard();


		atlas = gl2d::TextureAtlasPadding(8, 10, 8 * 8, 8 * 10);

		renderer.currentCamera.zoom = 60.f;
	
		return 1;
	}


	void update(pika::Input &input, pika::WindowState &windowState,
		GameplaySimulation &simulator)
	{
		{
			glClear(GL_COLOR_BUFFER_BIT);
			gl2d::enableNecessaryGLFeatures();
			renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
		}

		float wheel = ImGui::GetIO().MouseWheel;

		if ((ImGui::GetIO().KeysData[ImGuiKey_LeftCtrl].Down || ImGui::GetIO().KeysData[ImGuiKey_RightCtrl].Down) && input.hasFocus)
		{
			renderer.currentCamera.zoom += wheel * 3;
		}

		renderer.currentCamera.zoom = std::min(renderer.currentCamera.zoom, 70.f);
		renderer.currentCamera.zoom = std::max(renderer.currentCamera.zoom, 50.f);

	
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

					renderer.renderRectangle({i, j, 1, 1}, tiles, Colors_White, {}, 0, uv);

				}
		}


	}

	void followPlayer(mario::Player &p, pika::Input &input, pika::WindowState &windowState)
	{
		renderer.currentCamera.follow(p.position.getCenter(), input.deltaTime * 3, 0.0001, 0.2, windowState.windowW, windowState.windowH);


	}

	void drawPlayer(mario::Player &p)
	{
		glm::vec4 pos(p.position.position, 1, 1);
		//pos.y -= 1 / 8.f;
		pos.x -= 1 / 8.f;
		renderer.renderRectangle(pos, marioTexture, Colors_White, {}, 0.f, 
			p.movingRight ? glm::vec4(0, 1, 1, 0) : glm::vec4(1, 1, 0, 0));
	}

	void render()
	{
		renderer.flush();

	}

	void cleanup()
	{
		renderer.cleanup();
		marioTexture.cleanup();
		tiles.cleanup();
	}

};

static constexpr int visionSizeX = 7;
static constexpr int visionSizeY = 15;
static constexpr int subLayerSize = 15;

static constexpr int visionTotal = visionSizeX * visionSizeY;

inline float getRandomFloat(std::mt19937 &rng, float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

inline int getRandomInt(std::mt19937 &rng, int min, int max)
{
	std::uniform_int_distribution<int> dist(min, max);
	int rez = dist(rng);
	return rez;
}

inline bool getRandomChance(std::mt19937 &rng, float chance)
{
	float rez = getRandomFloat(rng, 0, 1);

	if (rez > chance)
	{
		return 0;
	}
	else
	{
		return 1;
	}

	//int chanceI = glm::clamp(chance, 0.f, 1.f) * 10000;
	//int pick = getRandomInt(rng, 0, 10000);
	//
	//if (pick > chanceI)
	//{
	//	return 0;
	//}
	//else
	//{
	//	return 1;
	//}
}

struct NeuralNetork
{



	float weights1[subLayerSize][visionTotal] = {};
	float weights2[subLayerSize][subLayerSize] = {};
	float weights3[3][subLayerSize] = {};


	float activationFunction(float a)
	{
		float sigmoid = 1.f / (1 + std::expf(-a));
		return (sigmoid * 2.f) - 1.f;
	}

	void compute(int &moveDirection, bool &jump, char input[visionTotal])
	{

		float rezult[3] = {};
		//for (int i = 0; i < 3; i++)
		//{
		//	for (int j = 0; j < visionTotal; j++)
		//	{
		//		if (input[j] > 0)
		//		{
		//			rezult[i] += weights[i][j];
		//		}
		//		else
		//		{
		//			rezult[i] -= weights[i][j];
		//		}
		//	}
		//}


		//first layer
		float resultFirstLayer[subLayerSize] = {};

		for (int i = 0; i < subLayerSize; i++)
		{

			float rez = 0;
			for (int j = 0; j < visionTotal; j++)
			{
				if (input[j] > 0)
				{
					rez += weights1[i][j];
				}
				else
				{
					rez -= weights1[i][j];
				}
			}
			rez = activationFunction(rez);
			resultFirstLayer[i] = rez;
		}

		resultFirstLayer[subLayerSize - 1] = 1;

		//second layer
		float resultSecondLayer[subLayerSize] = {};
		for (int i = 0; i < subLayerSize; i++)
		{

			float rez = 0;
			for (int j = 0; j < subLayerSize; j++)
			{
				rez += resultFirstLayer[j] * weights2[i][j];
			}

			rez = activationFunction(rez);
			resultSecondLayer[i] = rez;
		}


		resultSecondLayer[subLayerSize - 1] = 1;

		//final layer
		for (int i = 0; i < 3; i++)
		{
			float rez = 0;
			for (int j = 0; j < subLayerSize; j++)
			{
				rez += resultSecondLayer[j] * weights3[i][j];
			}

			rezult[i] = activationFunction(rez);

		}



		float dir = rezult[2] - rezult[1];
		if (std::abs(dir) < 0.2) { moveDirection = 0; }
		else if(dir > 0)
		{
			moveDirection = 1;
		}
		else
		{
			moveDirection = -1;
		}

		if (rezult[0] > 0)
		{
			jump = 1;
		}
		else
		{
			jump = 0;
		}

	}

	void addRandomConnection(std::mt19937 &rng)
	{
		std::vector<glm::ivec2> positions1;
		positions1.reserve(visionTotal* subLayerSize);
		for (int i = 0; i < subLayerSize; i++)
			for (int j = 0; j < visionTotal; j++)
			{
				if (weights1[i][j] == 0)
				{
					positions1.push_back({i,j});
				}
			}

		std::vector<glm::ivec2> positions2;
		positions2.reserve(subLayerSize * subLayerSize);
		for (int i = 0; i < subLayerSize; i++)
			for (int j = 0; j < subLayerSize; j++)
			{
				if (weights2[i][j] == 0)
				{
					positions2.push_back({i,j});
				}
			}

		std::vector<glm::ivec2> positions3;
		positions3.reserve(3 * subLayerSize);
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < subLayerSize; j++)
			{
				if (weights3[i][j] == 0)
				{
					positions3.push_back({i,j});
				}
			}


		if (!positions1.empty() && getRandomChance(rng, 0.6))
		{
			auto index = getRandomInt(rng, 0, positions1.size() - 1);
			weights1[positions1[index].x][positions1[index].y] = getRandomFloat(rng, -1.5, 1.5);
		}

		if (!positions2.empty() && getRandomChance(rng, 0.2))
		{
			auto index = getRandomInt(rng, 0, positions2.size() - 1);
			weights2[positions2[index].x][positions2[index].y] = getRandomFloat(rng, -1.5, 1.5);
		}

		if (!positions3.empty() && getRandomChance(rng, 0.2))
		{
			auto index = getRandomInt(rng, 0, positions3.size() - 1);
			weights3[positions3[index].x][positions3[index].y] = getRandomFloat(rng, -1.5, 1.5);
		}
	}

	
	void removeRandomWeight(std::mt19937 &rng)
	{
		std::vector<glm::ivec2> positions1;
		positions1.reserve(visionTotal * subLayerSize);
		for (int i = 0; i < subLayerSize; i++)
			for (int j = 0; j < visionTotal; j++)
			{
				if (weights1[i][j] != 0)
				{
					positions1.push_back({i,j});
				}
			}

		std::vector<glm::ivec2> positions2;
		positions2.reserve(subLayerSize * subLayerSize);
		for (int i = 0; i < subLayerSize; i++)
			for (int j = 0; j < subLayerSize; j++)
			{
				if (weights2[i][j] != 0)
				{
					positions2.push_back({i,j});
				}
			}

		std::vector<glm::ivec2> positions3;
		positions3.reserve(3 * subLayerSize);
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < subLayerSize; j++)
			{
				if (weights3[i][j] != 0)
				{
					positions3.push_back({i,j});
				}
			}

		if (!positions1.empty() && getRandomChance(rng, 0.6))
		{
			auto index = getRandomInt(rng, 0, positions1.size() - 1);
			weights1[positions1[index].x][positions1[index].y] = 0;
		}

		if (!positions2.empty() && getRandomChance(rng, 0.2))
		{
			auto index = getRandomInt(rng, 0, positions2.size() - 1);
			weights2[positions2[index].x][positions2[index].y] = 0;
		}

		if (!positions3.empty() && getRandomChance(rng, 0.2))
		{
			auto index = getRandomInt(rng, 0, positions3.size() - 1);
			weights3[positions3[index].x][positions3[index].y] = 0;
		}

	}

	void changeRandomWeight(std::mt19937 &rng)
	{
		std::vector<glm::ivec2> positions1;
		positions1.reserve(visionTotal * subLayerSize);
		for (int i = 0; i < subLayerSize; i++)
			for (int j = 0; j < visionTotal; j++)
			{
				if (weights1[i][j] != 0)
				{
					positions1.push_back({i,j});
				}
			}

		std::vector<glm::ivec2> positions2;
		positions2.reserve(subLayerSize * subLayerSize);
		for (int i = 0; i < subLayerSize; i++)
			for (int j = 0; j < subLayerSize; j++)
			{
				if (weights2[i][j] != 0)
				{
					positions2.push_back({i,j});
				}
			}

		std::vector<glm::ivec2> positions3;
		positions3.reserve(3 * subLayerSize);
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < subLayerSize; j++)
			{
				if (weights3[i][j] != 0)
				{
					positions3.push_back({i,j});
				}
			}

		if (!positions1.empty() && getRandomChance(rng, 0.6))
		{
			auto index = getRandomInt(rng, 0, positions1.size() - 1);
			weights1[positions1[index].x][positions1[index].y] = getRandomFloat(rng, -1.5, 1.5);
		}

		if (!positions2.empty() && getRandomChance(rng, 0.2))
		{
			auto index = getRandomInt(rng, 0, positions2.size() - 1);
			weights2[positions2[index].x][positions2[index].y] = getRandomFloat(rng, -1.5, 1.5);
		}

		if (!positions3.empty() && getRandomChance(rng, 0.2))
		{
			auto index = getRandomInt(rng, 0, positions3.size() - 1);
			weights3[positions3[index].x][positions3[index].y] = getRandomFloat(rng, -1.5, 1.5);
		}
	}

	void combine(std::mt19937 &rng, NeuralNetork &other)
	{
		//std::vector<glm::ivec2> positions;
		//for (int i = 0; i < 3; i++)
		//	for (int j = 0; j < visionTotal; j++)
		//	{
		//		if (weights[i][j] != other.weights[i][j])
		//		{
		//			positions.push_back({i,j});
		//		}
		//	}
		//
		//float chance = getRandomFloat(rng, 0, 1);
		//
		//for (auto p : positions)
		//{
		//	bool combine = getRandomInt(rng, 0, 5) == 1;
		//	int i = p.x;
		//	int j = p.y;
		//
		//	if (combine)
		//	{
		//		weights[i][j] = (weights[i][j] + other.weights[i][j]) / 2.f;
		//	}
		//	else
		//	{
		//		bool keeper = getRandomChance(rng, chance);
		//		if (keeper)
		//		{
		//			weights[i][j] = other.weights[i][j];
		//		}
		//		else
		//		{
		//			//keep
		//		}
		//	}
		//}


		//last layer
		for (int i = 0; i < 3; i++)
		{
			bool keep = getRandomChance(rng, 0.5);

			if (!keep)
			{
				for (int j = 0; j < subLayerSize; j++)
				{
					weights3[i][j] = other.weights3[i][j];
				}
			}
		}

		//intermediate layer
		for (int i = 0; i < subLayerSize; i++)
		{
			bool keep = getRandomChance(rng, 0.5);

			if (!keep)
			{
				for (int j = 0; j < subLayerSize; j++)
				{
					weights2[i][j] = other.weights2[i][j];
				}
			}
		}

		//first layer
		for (int i = 0; i < subLayerSize; i++)
		{
			bool keep = getRandomChance(rng, 0.5);

			if (!keep)
			{
				for (int j = 0; j < visionTotal; j++)
				{
					weights1[i][j] = other.weights1[i][j];
				}
			}
		}


	}

};

struct PlayerSimulation
{
	mario::Player p;
	float maxFit = 0;
	float maxPosition = 0;
	float killTimer = 0;
	int jumpCount = 0;
};

bool performNeuralSimulation(PlayerSimulation &p, float deltaTime, mario::GameplaySimulation &simulator, mario::NeuralNetork
	&network);

void renderNeuralNetwork(gl2d::Renderer2D &renderer, char vision[mario::visionSizeX * mario::visionSizeY], 
	float blockSizePreview, mario::NeuralNetork &network);

void getVision(char vision[visionSizeX * visionSizeY], mario::GameplaySimulation &simulator, PlayerSimulation &p);


};
