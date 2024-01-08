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

	itemsSprite = pika::gl2d::loadTexture(PIKA_RESOURCES_PATH "iso/tiles/items.png",
		requestedInfo, true);
	itemsAtlas = gl2d::TextureAtlas(3, 1);

	itemFrameSprite = pika::gl2d::loadTexture(PIKA_RESOURCES_PATH "iso/tiles/ui.png",
		requestedInfo, true);


	font = pika::gl2d::loadFont(PIKA_RESOURCES_PATH "mcDungeons/CommodorePixeled.ttf", requestedInfo);


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

	return true;
}

static bool canPlayerStay(int type, int redstone)
{
	return type == 0 || type == IsometricGameEditor::Blocks::redstone ||
		type == IsometricGameEditor::Blocks::redstoneTorch ||
		type == IsometricGameEditor::Blocks::lever ||
	 	(type == IsometricGameEditor::Blocks::trapdor && redstone == 1);
}

bool IsometricGame::update(pika::Input input, pika::WindowState windowState, RequestedContainerInfo &requestedInfo)
{

#pragma region clear stuff
	glClear(GL_COLOR_BUFFER_BIT);
	renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
#pragma endregion

	float size = 100;

#pragma region Change Level
	{
		
		if (currentLevel == 0)
		{
			if(
				playerPosition == glm::ivec3{0,1,10} ||
				playerPosition == glm::ivec3{0,1,11}
				)
			{currentLevel = 1; playerPosition = glm::ivec3(8, 1, 4);}
			else
			if(
				playerPosition == glm::ivec3{4,1,6} ||
				playerPosition == glm::ivec3{3,1,6}
				)
			{currentLevel = 2; playerPosition = glm::ivec3(3, 1, 7); };
		}
		else if (currentLevel == 1)
		{
			if (
				playerPosition == glm::ivec3{0,1,4}
				){currentLevel = 3; playerPosition = glm::ivec3(8, 1, 2); }
			else if (playerPosition == glm::ivec3(9, 1, 4) ||
				playerPosition == glm::ivec3(9, 1, 5)
				)
			{
				currentLevel = 0; playerPosition = glm::ivec3(1, 1, 10);
			}

		}
		else if (currentLevel == 2)
		{
			if (
				playerPosition == glm::ivec3{3,1,9} ||
				playerPosition == glm::ivec3{2,1,9}
				)
			{
				currentLevel = 0; playerPosition = glm::ivec3(4, 1, 7);
			}
		}
		else if (currentLevel == 3)
		{

			if (
				playerPosition == glm::ivec3{9,1,2} ||
				playerPosition == glm::ivec3{9,1,3}
				)
			{
				currentLevel = 1; playerPosition = glm::ivec3(1, 1, 4);
			}else
			if (
				playerPosition == glm::ivec3{1,1,16} ||
				playerPosition == glm::ivec3{0,1,16} ||
				playerPosition == glm::ivec3{0,1,15}
				)
			{
				currentLevel = 4; playerPosition = glm::ivec3(13, 1, 3);
			}
		}
		else if (currentLevel == 4)
		{

			if (
				playerPosition == glm::ivec3{14,1,3} ||
				playerPosition == glm::ivec3{14,1,4}
				)
			{
				currentLevel = 3; playerPosition = glm::ivec3(2, 1, 16); //?
			}
			else if(playerPosition == glm::ivec3{6,1,14} ||
				playerPosition == glm::ivec3{5,1,14})
			{
				return 0;
			}
		}

	}
#pragma endregion

	
	
	IsometricGameEditor::Map &map = levels[currentLevel];

#pragma region Redstone

	auto getRedstoneStatusUnsafe = [&](glm::ivec3 v) -> RedstoneStatus &
	{
		return redstone[v.x * map.size.z * map.size.y + v.y * map.size.z + v.z];
	};

	{
		redstone.clear();
		redstone.resize(map.size.x * map.size.y * map.size.z);

		std::vector<glm::ivec3> emitors;

		for (int x = 0; x < map.size.x; x++)
			for (int y = 0; y < map.size.y; y++)
				for (int z = 0; z < map.size.z; z++)
				{
					auto b = map.getSafe({x,y,z});

					if (b->get().x == IsometricGameEditor::Blocks::redstoneBlock ||
						b->get().x == IsometricGameEditor::Blocks::redstoneTorch ||
						(b->get().x == IsometricGameEditor::Blocks::lever && b->get().y == 1)
						)
					{
						emitors.emplace_back(x, y, z);
						getRedstoneStatusUnsafe({x, y, z}).status = 1;
					}
				}

		while (!emitors.empty())
		{
			auto emitor = emitors.back();
			emitors.pop_back();

			auto check = [&](glm::ivec3 pos)
			{
				auto b = map.getSafe(pos);
				if(b)
				{
					if (getRedstoneStatusUnsafe(pos).status == 0)
					{
						getRedstoneStatusUnsafe(pos).status = 1;
						if (b->get().x == IsometricGameEditor::Blocks::redstone)
						{
							emitors.push_back(pos);
						}
					}
				}
			};

			check(emitor + glm::ivec3(1, 0, 0));
			check(emitor + glm::ivec3(-1, 0, 0));
			check(emitor + glm::ivec3(0, 1, 0));
			check(emitor + glm::ivec3(0, -1, 0));
			check(emitor + glm::ivec3(0, 0, 1));
			check(emitor + glm::ivec3(0, 0, -1));
		}

	}
#pragma endregion


#pragma region UI1

	glm::vec4 uiBox = {};
	{
		glui::Frame f({0, 0, windowState.windowW, windowState.windowH});
		uiBox = glui::Box().xLeftPerc(0.05).yTopPerc(0.02)
			.xDimensionPercentage(0.4).yDimensionPercentage(0.2);

		float boxSize = std::min(uiBox.z / 3.f, uiBox.w);
		uiBox.z = boxSize * 3;
		uiBox.w = boxSize;

	}

#pragma endregion


	auto calculateBlockPos = [size](glm::vec3 in)
	{
		glm::vec2 position = {};

		position += glm::vec2(-size / 2.f, size / 4.f) * float(in.x);
		position += glm::vec2(size / 2.f, size / 4.f) * float(in.z);

		position += glm::vec2(0, -size / 2.f) * float(in.y);

		return position;
	};

#pragma region camera

	//renderer.currentCamera.follow(calculateBlockPos({map.size.x / 2-1, 1, map.size.z / 2}),
	//	100, 0, 0, windowState.windowW, windowState.windowH);

	renderer.currentCamera.follow(calculateBlockPos({glm::vec3(playerPosition) + playerAnimations.delta}),
		input.deltaTime*180, 1, 90, windowState.windowW, windowState.windowH);

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

							if (IsometricGameEditor::pointInBox(glm::vec2(input.mouseX, input.mouseY), box)
								&& !IsometricGameEditor::pointInBox(glm::vec2(input.mouseX, input.mouseY), uiBox)
								)
							{
								float manhatan1 = glm::dot((glm::vec2(input.mouseX, input.mouseY) - glm::vec2(box)), glm::vec2(1,1));
								float manhatan2 = glm::dot((glm::vec2(box) + glm::vec2(box.z, box.w)
									- glm::vec2(input.mouseX, input.mouseY)), glm::vec2(1, 1));

								if (manhatan1 > size / 4)
								if (manhatan2 > size / 4)
								{

									return true;
								}

							}

						}

						return false;
					};

					if (checkPointInBox(x,y,z))
					{
						
						//if (y != 1 && canPlayerStay(map.getSafe({x,y,z}))) {}

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
			if (canPlayerStay(map.getSafe(currentSelectedBlockDelete)->get().x, 
				getRedstoneStatusUnsafe(currentSelectedBlockDelete).status)
				)
			{
				currentBlockMove = currentSelectedBlockDelete;
			}
			else
			{
				currentBlockInteract = currentSelectedBlockDelete;
			}
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
					auto cr = glm::vec4(0.4, 0.2, 0.1, 1.0);


					if (b->get().x == IsometricGameEditor::Blocks::redstone)
					{
						if (getRedstoneStatusUnsafe({x,y,z}).status == 1)
						{
							color = glm::vec4(0.9, 0.25, 0.25, 1.0);
							cr = glm::vec4(0.9, 0.25, 0.25, 1.0);
						}
						else
						{
							color = glm::vec4(0.4, 0.1, 0.1, 1.0);
							cr = glm::vec4(0.4, 0.1, 0.1, 1.0);
						}
					}

					if (b->get().x == IsometricGameEditor::Blocks::redstone || b->get().x == IsometricGameEditor::Blocks::lever
						|| b->get().x == IsometricGameEditor::Blocks::redstoneTorch
						)
					{

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

					if (b->get().x == IsometricGameEditor::Blocks::trapdor)
					{
						renderer.renderRectangle({position,size,size}, tiles, {1,1,1,1}, {}, 0,
							tilesAtlas.get(b->get().x, getRedstoneStatusUnsafe({x,y,z}).status)
						);
					}
					else
					{
						renderer.renderRectangle({position,size,size}, tiles, color, {}, 0,
							tilesAtlas.get(b->get().x, b->get().y));
					}


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

		if (path.empty())
		{
			playerAnimations.indexY = 0;
		}
		else
		{
			playerAnimations.indexY = 2;
		}

		if (playerAnimations.timer <= 0)
		{
			playerAnimations.timer += 0.2;
			playerAnimations.indexX += 1;
			if (playerAnimations.indexX >= 5)
			{
				playerAnimations.indexX = 0;
			}
		}

		renderer.renderRectangle({calculateBlockPos(glm::vec3(playerPosition) + playerAnimations.delta),size,size}, playerSprite,
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
					playerAnimations.lastPosition = currentBlock;

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
					if (b && canPlayerStay(b->get().x, 
						getRedstoneStatusUnsafe({currentBlock.x + x, 1, currentBlock.y + z}).status)
						)
					{
						if (searcehedPositions.find({currentBlock.x + x, currentBlock.y + z})
							== searcehedPositions.end())
						{
							searcehedPositions[glm::ivec2{currentBlock.x + x, currentBlock.y + z}] 
								= glm::ivec2{currentBlock.x, currentBlock.y};
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

		
		while (timerPath <= 0)
		{
			timerPath += 0.2;
			playerPosition = glm::vec3{path[0].x, 1, path[0].y};
			playerAnimations.lastPosition = glm::ivec2(playerPosition.x, playerPosition.z);
			path.erase(path.begin());
		}

		{
			glm::vec2 delta = glm::mix(
				glm::vec2(path[0].x, path[0].y),
				glm::vec2(playerAnimations.lastPosition),
				timerPath / 0.2f) - glm::vec2(playerAnimations.lastPosition);
			playerAnimations.delta = {delta.x, 0, delta.y};
		}

		//reached end
		if (path.empty())
		{
			auto b = map.getSafe(playerPosition);

			if (b->get().x == IsometricGameEditor::Blocks::lever)
			{
				//flip lever
				b->secondType = !b->get().y;
			}
			else if (b->get().x == IsometricGameEditor::Blocks::redstone)
			{
				b->set(0,0);
				redstoneCount++;
			}
			else if (b->get().x == IsometricGameEditor::Blocks::redstoneTorch)
			{
				b->set(0, 0);
				redstoneTorchesCount++;
			}
			else if (b->get().x == 0)
			{
				if (itemSelected == 0 && redstoneCount)
				{
					itemSelected = -1;
					redstoneCount--;
					b->set(IsometricGameEditor::Blocks::redstone, 0);
				}else 
				if (itemSelected == 1 && redstoneTorchesCount)
				{
					itemSelected = -1;
					redstoneTorchesCount--;
					b->set(IsometricGameEditor::Blocks::redstoneTorch, 0);
				}
			}
		}

	}


#pragma endregion

#pragma region UI2

	renderer.pushCamera();
	{
		glui::Frame f(uiBox);
		renderer.renderRectangle(uiBox, {0.1,0.1,0.1,0.5});
		
		float boxSize = uiBox.z/3.f;

		renderer.renderRectangle({uiBox.x, uiBox.y, boxSize, boxSize}, itemsSprite, {1,1,1,0.9},
			{}, 0.f, itemsAtlas.get(0,0));

		renderer.renderRectangle({uiBox.x + boxSize, uiBox.y, boxSize, boxSize}, itemsSprite, {1,1,1,0.9},
			{}, 0.f, itemsAtlas.get(1, 0));

		renderer.renderRectangle({uiBox.x + boxSize * 2, uiBox.y, boxSize, boxSize}, itemsSprite, {1,1,1,0.9},
			{}, 0.f, itemsAtlas.get(2, 0));

		if (itemSelected != -1)
		{
			renderer.renderRectangle({uiBox.x + boxSize * itemSelected, uiBox.y, boxSize, boxSize}, itemFrameSprite);
		}

		renderer.renderText({uiBox.x + boxSize*0.75, uiBox.y + boxSize * 0.75},
			std::to_string(redstoneCount).c_str(), font, Colors_White);

		renderer.renderText({uiBox.x + boxSize * 1.75, uiBox.y + boxSize * 0.75},
			std::to_string(redstoneTorchesCount).c_str(), font, Colors_White);

		renderer.renderText({uiBox.x + boxSize * 2.75, uiBox.y + boxSize * 0.75},
			std::to_string(foodCount).c_str(), font, Colors_White);


		if (input.lMouse.pressed())
		{
			if (IsometricGameEditor::pointInBox(glm::vec2(input.mouseX, input.mouseY), {uiBox.x, uiBox.y, boxSize, boxSize}))
			{
				itemSelected = 0;
			}

			if (IsometricGameEditor::pointInBox(glm::vec2(input.mouseX, input.mouseY), {uiBox.x + boxSize, uiBox.y, boxSize, boxSize}))
			{
				itemSelected = 1;
			}

			if (IsometricGameEditor::pointInBox(glm::vec2(input.mouseX, input.mouseY), {uiBox.x + boxSize * 2, uiBox.y, boxSize, boxSize}))
			{
				itemSelected = 2;
			}
		}

		
	}
	renderer.popCamera();

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
