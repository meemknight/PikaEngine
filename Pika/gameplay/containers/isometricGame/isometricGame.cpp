#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include <containers/isometricGame/isometricGame.h>
#include <containers/isometricGame/isometricGameEditor.h>
#include <glui/glui.h>
#include <deque>
#include <unordered_map>

bool IsometricGame::create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
{
	renderer.create(requestedInfo.requestedFBO.fbo);

	tiles = pika::gl2d::loadTextureWithPixelPadding(PIKA_RESOURCES_PATH "iso/tiles/Isometric-Tiles.png", requestedInfo, 32, true);
	shadow = pika::gl2d::loadTexture(PIKA_RESOURCES_PATH "iso/tiles/shadow.png", requestedInfo, 32, true);
	tilesAtlas = gl2d::TextureAtlasPadding(16, 8, tiles.GetSize().x, tiles.GetSize().y);

	playerSprite = pika::gl2d::loadTexture(PIKA_RESOURCES_PATH "iso/tiles/IsoCharacter.png",
		requestedInfo, true);
	playerAtlas = gl2d::TextureAtlas(5, 4);


	for (int i = 0; i < MAPS_COUNT; i++)
	{
		//editor.loadFromFile(renderer, commandLineArgument.to_string(), requestedInfo);
		size_t s = 0;
		std::string fileName = PIKA_RESOURCES_PATH "iso/maps/map" + std::to_string(i+1) + ".isomap";

		glm::ivec3 mapSize = {};
		bool created = 0;
		if (requestedInfo.readEntireFileBinary(fileName, &mapSize, sizeof(mapSize), 0))
		{
			if (requestedInfo.getFileSizeBinary(fileName, s))
			{

				if (s == mapSize.x * mapSize.y * mapSize.z * sizeof(Block) + sizeof(mapSize))
				{
					created = 1;

					levels[i].init(mapSize);

					requestedInfo.readEntireFileBinary(fileName,
						levels[i].mapData.data(), mapSize.x * mapSize.y * mapSize.z * sizeof(Block), sizeof(mapSize));

				}
			}
		}

		if (!created)
		{
			requestedInfo.consoleWrite("Error loading file: " + std::to_string(i) + "\n");
			return 0;
		}
			
	}

	map = levels[0];
	
	return true;
}

static bool canPlayerStay(int type)
{
	return type == 0 || type == IsometricGameEditor::Blocks::redstone ||
		type == IsometricGameEditor::Blocks::lever;
}

bool IsometricGame::update(pika::Input input, pika::WindowState windowState, RequestedContainerInfo &requestedInfo)
{

#pragma region clear stuff
	glClear(GL_COLOR_BUFFER_BIT);
	renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
#pragma endregion

	float size = 100;
	
#pragma region move

	float cameraSpeed = input.deltaTime * size * 2.f;

	if (input.buttons[pika::Button::A].held())
	{
		renderer.currentCamera.position.x -= cameraSpeed;
	}
	if (input.buttons[pika::Button::D].held())
	{
		renderer.currentCamera.position.x += cameraSpeed;
	}

	if (input.buttons[pika::Button::W].held())
	{
		renderer.currentCamera.position.y -= cameraSpeed;
	}
	if (input.buttons[pika::Button::S].held())
	{
		renderer.currentCamera.position.y += cameraSpeed;
	}
#pragma endregion


	auto calculateBlockPos = [size](glm::ivec3 in)
	{
		glm::vec2 position = {};

		position += glm::vec2(-size / 2.f, size / 4.f) * float(in.x);
		position += glm::vec2(size / 2.f, size / 4.f) * float(in.z);

		position += glm::vec2(0, -size / 2.f) * float(in.y);

		return position;
	};

#pragma region camera

	renderer.currentCamera.follow(calculateBlockPos({map.size.x / 2-1, 1, map.size.z / 2}),
		100, 0, 0, windowState.windowW, windowState.windowH);

#pragma endregion




	glm::ivec3 currentSelectedBlockDelete{-1};
	glm::ivec3 currentSelectedBlockPlace{-1};

	glm::ivec3 currentBlockInteract{-1};
	glm::ivec3 currentBlockMove{-1};

	auto viewRect = renderer.getViewRect();

	auto lerp = [](auto a, auto b, auto c)
	{
		return a * (1.f - c) + b * c;
	};

	glm::vec2 blockPositionScreen = lerp(glm::vec2(viewRect.x, viewRect.y),
		glm::vec2(viewRect.x + viewRect.z, viewRect.y + viewRect.w),
		glm::vec2(input.mouseX, input.mouseY) / glm::vec2(windowState.windowW, windowState.windowH));


		for (int y = 0; y < map.size.y; y++)
			for (int z = 0; z < map.size.z; z++)
				for (int x = 0; x < map.size.x; x++)
				{

					auto checkPointInBox = [&](int x, int y, int z, bool reverse = 0)
					{
						auto b = map.getSafe({x,y,z});

						if (!b) { return false; }

						if ((b->get().x != 0 && !reverse)
							|| (b->get().x == 0 && reverse)
							)
						{
							glm::vec2 position = calculateBlockPos({x,y,z});
							glm::vec4 box = renderer.toScreen({position, size, size});
							//todo new functon

							box.x += 1;
							box.x /= 2.f;
							box.x *= renderer.windowW;

							box.y *= -1;
							box.y += 1;
							box.y /= 2.f;
							box.y *= renderer.windowH;

							box.z += 1;
							box.z /= 2.f;
							box.z *= renderer.windowW;

							box.w *= -1;
							box.w += 1;
							box.w /= 2.f;
							box.w *= renderer.windowH;

							box.z = box.z - box.x;
							box.w = box.w - box.y;

							if (IsometricGameEditor::pointInBox(glm::vec2(input.mouseX, input.mouseY), box))
							{
								return true;
							}

						}

						return false;
					};

					if (checkPointInBox(x,y,z))
					{
						currentSelectedBlockDelete = {x,y,z};

						if (checkPointInBox(x + 1, y, z, true))
						{
							currentSelectedBlockPlace = {x + 1,y,z};
						}

						if (checkPointInBox(x, y + 1, z, true))
						{
							currentSelectedBlockPlace = {x,y + 1,z};
						}

						if (checkPointInBox(x, y, z + 1, true))
						{
							currentSelectedBlockPlace = {x,y,z + 1};
						}
					}

				}

		if (currentSelectedBlockDelete.y == 0)
		{
			if (currentSelectedBlockPlace == currentSelectedBlockDelete + glm::ivec3(0, 1, 0))
			{
				currentBlockMove = currentSelectedBlockPlace;
			}
		}
		else if (currentSelectedBlockDelete.y == 1)
		{
			currentBlockInteract = currentSelectedBlockDelete;
		}
		else
		{
			currentSelectedBlockDelete = glm::vec3{-1};
			currentSelectedBlockPlace = glm::vec3{-1};
		}

	for (int y = 0; y < map.size.y; y++)
		for (int z = 0; z < map.size.z; z++)
			for (int x = 0; x < map.size.x; x++)
			{
				auto b = map.getSafe({x,y,z});

				if (b->get().x != 0)
				{
					glm::vec2 position = calculateBlockPos({x,y,z});

					glm::vec4 color = Colors_White;

					if (b->get().x == IsometricGameEditor::Blocks::redstone)
					{
						color = glm::vec4(0.8, 0.2, 0.2, 1.0);
					}

					if (b->get().x == IsometricGameEditor::Blocks::redstone || b->get().x == IsometricGameEditor::Blocks::lever
						|| b->get().x == IsometricGameEditor::Blocks::redstoneTorch
						)
					{
						auto cr = glm::vec4(0.8, 0.2, 0.2, 1.0);

						auto b2 = map.getSafe({x,y,z - 1});
						if (b2 && IsometricGameEditor::redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(IsometricGameEditor::Blocks::redstone, 1));
						}

						b2 = map.getSafe({x - 1,y,z});
						if (b2 && IsometricGameEditor::redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(IsometricGameEditor::Blocks::redstone, 2));
						}

						b2 = map.getSafe({x + 1,y,z});
						if (b2 && IsometricGameEditor::redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(IsometricGameEditor::Blocks::redstone, 3));
						}

						b2 = map.getSafe({x,y,z + 1});
						if (b2 && IsometricGameEditor::redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(IsometricGameEditor::Blocks::redstone, 4));
						}

//redstone on wall

b2 = map.getSafe({x - 1,y + 1,z});
if (b2 && IsometricGameEditor::redstoneWire(b2->type))
{
	renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
		tilesAtlas.get(IsometricGameEditor::Blocks::redstone, 2));

	renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
		tilesAtlas.get(IsometricGameEditor::Blocks::redstone, 5));
}

b2 = map.getSafe({x,y + 1,z - 1});
if (b2 && IsometricGameEditor::redstoneWire(b2->type))
{
	renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
		tilesAtlas.get(IsometricGameEditor::Blocks::redstone, 1));

	renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
		tilesAtlas.get(IsometricGameEditor::Blocks::redstone, 6));
}


//redstone down

b2 = map.getSafe({x + 1,y - 1,z});
if (b2 && IsometricGameEditor::redstoneWire(b2->type))
{
	renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
		tilesAtlas.get(IsometricGameEditor::Blocks::redstone, 3));
}

b2 = map.getSafe({x,y - 1,z + 1});
if (b2 && IsometricGameEditor::redstoneWire(b2->type))
{
	renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
		tilesAtlas.get(IsometricGameEditor::Blocks::redstone, 4));
}

					}

					renderer.renderRectangle({position,size,size}, tiles, color, {}, 0,
						tilesAtlas.get(b->get().x, b->get().y));

					int advance = 1;
					for (int y2 = y + 2; y2 < map.size.y; y2++)
					{
						advance++;

						auto b2 = map.getSafe({x,y2,z});

						if (b2 && b2->get().x != 0 && b->get().x != IsometricGameEditor::Blocks::redstone
							&& b->get().x != IsometricGameEditor::Blocks::lever)
						{
							renderer.renderRectangle({position,size,size}, shadow,
								{1,1,1,1.f / advance}
							);
							break;
						}

					}

				}

				//if (currentSelectedBlockDelete == glm::ivec3{x, y, z})
				//{
				//	glm::vec2 position = calculateBlockPos({x,y,z});
				//
				//	renderer.renderRectangle({position,size,size}, tiles, Colors_White, {}, 0,
				//		tilesAtlas.get(0, 1));
				//}

				//if (currentSelectedBlockPlace == glm::ivec3{x, y, z})
				//{
				//	glm::vec2 position = calculateBlockPos({x,y,z});
				//
				//	renderer.renderRectangle({position,size,size}, tiles, {0.1,0.1,0.1,0.1}, {}, 0,
				//		tilesAtlas.get(0, 1));
				//}

				if (currentBlockMove == glm::ivec3{x, y, z})
				{
					glm::vec2 position = calculateBlockPos({x,y,z});

					renderer.renderRectangle({position,size,size}, tiles, {0.5,0.1,0.1,0.6}, {}, 0,
						tilesAtlas.get(0, 1));
				}

				if (currentBlockInteract == glm::ivec3{x, y, z})
				{
					glm::vec2 position = calculateBlockPos({x,y,z});

					renderer.renderRectangle({position,size,size}, tiles, {1,0.1,0.1,0.6}, {}, 0,
						tilesAtlas.get(0, 1));
				}

			}


#pragma region render player
	{

		playerAnimations.timer -= input.deltaTime;

		if (playerAnimations.timer <= 0)
		{
			playerAnimations.timer += 0.2;
			playerAnimations.indexX += 1;
			if (playerAnimations.indexX >= 5)
			{
				playerAnimations.indexX = 0;
			}
		}

		renderer.renderRectangle({calculateBlockPos(playerPosition),size,size}, playerSprite,
			Colors_White, {}, 0.f, playerAtlas.get(playerAnimations.indexX, playerAnimations.indexY));

	}
#pragma endregion

#pragma region Move

	if (path.empty())
	{
		timerPath = 0.2;

		if (input.lMouse.pressed() && currentBlockMove.x >= 0)
		{
			std::deque<glm::ivec2> positionsToSearch;

			positionsToSearch.push_back({currentBlockMove.x,currentBlockMove.z});

			std::unordered_map<glm::ivec2, glm::ivec2> searcehedPositions;
			searcehedPositions[glm::ivec2{currentBlockMove.x, currentBlockMove.z}] = glm::ivec2{currentBlockMove.x,currentBlockMove.z};

			while (!positionsToSearch.empty())
			{
				auto currentBlock = positionsToSearch.front();
				positionsToSearch.pop_front();

				if (currentBlock == glm::ivec2{playerPosition.x,playerPosition.z})
				{
					currentBlock = searcehedPositions[currentBlock];

					while (true)
					{
						currentBlock = searcehedPositions[currentBlock];
						path.push_back(currentBlock);
						if (currentBlock == glm::ivec2{currentBlockMove.x, currentBlockMove.z})
						{
							break;
						}
					}

					break;
				}

				auto tryBlock = [&](int x, int z)
				{
					auto b = map.getSafe({currentBlock.x + x, 1, currentBlock.y + z});
					if (b && canPlayerStay(b->get().x) )
					{
						if (searcehedPositions.find({currentBlock.x + x, currentBlock.y + z})
							== searcehedPositions.end())
						{
							searcehedPositions[glm::ivec2{currentBlock.x + x, currentBlock.y + z}] = glm::ivec2{currentBlock.x, currentBlock.y};
							positionsToSearch.push_back({currentBlock.x + x, currentBlock.y + z});
						}
					}
				};

				tryBlock(1, 0);
				tryBlock(-1, 0);
				tryBlock(0, 1);
				tryBlock(0, -1);


			}

		}
	}
	else
	{
		timerPath -= input.deltaTime;

		if (timerPath < 0)
		{
			timerPath += 0.2;
			playerPosition = glm::vec3{path[0].x, 1, path[0].y};
			path.erase(path.begin());
		}
	}


#pragma endregion



	ImGui::Begin("Game debug");

	ImGui::DragInt3("player position", &playerPosition[0], 1, 0, 20);

	ImGui::End();

	renderer.flush();



	return true;
}

void IsometricGame::destruct(RequestedContainerInfo &requestedInfo)
{
	renderer.cleanup();

}
