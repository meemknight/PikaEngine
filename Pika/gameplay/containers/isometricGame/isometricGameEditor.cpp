#include <containers/isometricGame/isometricGameEditor.h>
#include <glui/glui.h>

static int blocksCount = 16;

static bool pointInBox(glm::vec2 p, glm::vec4 box)
{
	if
		(
		(p.x >= box.x && p.x <= (box.x + box.z)) &&
		(p.y >= box.y && p.y <= (box.y + box.w))
		)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static bool redstoneWire(int type)
{
	return type == IsometricGameEditor::Blocks::redstone ||
		type == IsometricGameEditor::Blocks::trapdor ||
		type == IsometricGameEditor::Blocks::redstoneTorch ||
		type == IsometricGameEditor::Blocks::redstoneBlock ||
	type == IsometricGameEditor::Blocks::lever;
}

static bool canPlaceRedstoneOn(int type)
{
	return (type >= IsometricGameEditor::Blocks::clay &&
		type <= IsometricGameEditor::Blocks::woddenPlank)
		|| type == IsometricGameEditor::Blocks::redstoneBlock;
		
}


bool IsometricGameEditor::create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
{
	renderer.create(requestedInfo.requestedFBO.fbo);

	tiles = pika::gl2d::loadTextureWithPixelPadding(PIKA_RESOURCES_PATH "isoTiles/Isometric-Tiles.png", requestedInfo, 32, true);
	shadow = pika::gl2d::loadTexture(PIKA_RESOURCES_PATH "isoTiles/shadow.png", requestedInfo, 32, true);

	tilesAtlas = gl2d::TextureAtlasPadding(16, 8, tiles.GetSize().x, tiles.GetSize().y);

	//fileChanged.setFile(mapFile.c_str());

	loadedLevel.setInfo("Level", PIKA_RESOURCES_PATH, {".mcDungeons"});

	bool created = 0;
	if (commandLineArgument.size() > 0)
	{
		//editor.loadFromFile(renderer, commandLineArgument.to_string(), requestedInfo);
		size_t s = 0;
		pika::strlcpy(loadedLevel.file, commandLineArgument.to_string(), sizeof(loadedLevel.file));

		glm::ivec3 mapSize = {};
		if (requestedInfo.readEntireFileBinary(commandLineArgument.to_string(), &mapSize, sizeof(mapSize), 0))
		{
			if (requestedInfo.getFileSizeBinary(commandLineArgument.to_string().c_str(), s))
			{

				if (s == mapSize.x * mapSize.y * mapSize.z * sizeof(Block) + sizeof(mapSize))
				{
					created = 1;

					map.init(mapSize);

					requestedInfo.readEntireFileBinary(commandLineArgument.to_string().c_str(), 
						map.mapData.data(), mapSize.x * mapSize.y * mapSize.z * sizeof(Block), sizeof(mapSize));

				}
			}


		}

		
	}

	if (!created)
	{
		map.init({10,10,10});

		for (int i = 0; i < 10; i++)
			for (int j = 0; j < 10; j++)
			{
				map.setSafe({i,0,j}, 1, 0);
			}

		map.setSafe({5,1,5}, 6, 0);
		map.setSafe({5,2,5}, 6, 0);
	}

	
	newMapSize = map.size;

	return true;
}

bool IsometricGameEditor::update(pika::Input input, pika::WindowState windowState, RequestedContainerInfo &requestedInfo)
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

	glm::ivec3 currentSelectedBlockDelete{-1};
	glm::ivec3 currentSelectedBlockPlace{-1};

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

							if (pointInBox(glm::vec2(input.mouseX, input.mouseY), box))
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

	for (int y = 0; y < map.size.y; y++)
		for (int z = 0; z < map.size.z; z++)
			for (int x = 0; x < map.size.x; x++)
			{
				auto b = map.getSafe({x,y,z});

				if (b->get().x != 0)
				{
					glm::vec2 position = calculateBlockPos({x,y,z});

					glm::vec4 color = Colors_White;

					if (b->get().x == Blocks::redstone)
					{
						color = glm::vec4(0.8, 0.2, 0.2, 1.0);
					}

					if (b->get().x == Blocks::redstone || b->get().x == Blocks::lever
						|| b->get().x == Blocks::redstoneTorch
						)
					{
						auto cr = glm::vec4(0.8, 0.2, 0.2, 1.0);

						auto b2 = map.getSafe({x,y,z - 1});
						if (b2 && redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(Blocks::redstone, 1));
						}

						b2 = map.getSafe({x - 1,y,z});
						if (b2 && redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(Blocks::redstone, 2));
						}

						b2 = map.getSafe({x + 1,y,z});
						if (b2 && redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(Blocks::redstone, 3));
						}

						b2 = map.getSafe({x,y,z + 1});
						if (b2 && redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(Blocks::redstone, 4));
						}

						//redstone on wall

						b2 = map.getSafe({x - 1,y + 1,z});
						if (b2 && redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(Blocks::redstone, 2));

							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(Blocks::redstone, 5));
						}

						b2 = map.getSafe({x,y + 1,z - 1});
						if (b2 && redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(Blocks::redstone, 1));

							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(Blocks::redstone, 6));
						}


						//redstone down

						b2 = map.getSafe({x + 1,y - 1,z});
						if (b2 && redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(Blocks::redstone, 3));
						}

						b2 = map.getSafe({x,y - 1,z + 1});
						if (b2 && redstoneWire(b2->type))
						{
							renderer.renderRectangle({position,size,size}, tiles, cr, {}, 0,
								tilesAtlas.get(Blocks::redstone, 4));
						}

					}

					renderer.renderRectangle({position,size,size}, tiles, color, {}, 0,
						tilesAtlas.get(b->get().x, b->get().y));

					int advance = 1;
					for (int y2 = y + 2; y2 < map.size.y; y2++)
					{
						advance++;

						auto b2 = map.getSafe({x,y2,z});

						if (b2 && b2->get().x != 0 && b->get().x != Blocks::redstone 
							&& b->get().x != Blocks::lever)
						{
							renderer.renderRectangle({position,size,size}, shadow, 
								{1,1,1,1.f/advance}
							);
							break;
						}

					}

				}

				if (currentSelectedBlockDelete == glm::ivec3{x, y, z})
				{
					glm::vec2 position = calculateBlockPos({x,y,z});
				
					renderer.renderRectangle({position,size,size}, tiles, Colors_White, {}, 0,
						tilesAtlas.get(0, 1));
				}

				if (currentSelectedBlockPlace == glm::ivec3{x, y, z})
				{
					glm::vec2 position = calculateBlockPos({x,y,z});

					renderer.renderRectangle({position,size,size}, tiles, {0.1,0.1,0.1,0.1}, {}, 0,
						tilesAtlas.get(0, 1));
				}

				if (blockSelector == glm::ivec3{x, y, z})
				{
					glm::vec2 position = calculateBlockPos({x,y,z});

					renderer.renderRectangle({position,size,size}, tiles, {0.5,0.9,0.1,0.9}, {}, 0,
						tilesAtlas.get(0, 1));
				}
			}

	if (currentSelectedBlockDelete.x > -1)
	{
		if (
			input.lMouse.pressed()
			)
		{
			auto b = map.getSafe(currentSelectedBlockDelete);
			if (b)
			{
				b->set(0, 0);
			}
		}
	}

	if (currentSelectedBlockPlace.x > -1)
	{
		if (
			input.rMouse.pressed()
			)
		{
			auto b = map.getSafe(currentSelectedBlockPlace);
			if (b)
			{
				if (currentBlock == Blocks::redstone || currentBlock == Blocks::trapdor)
				{
					auto b2 = map.getSafe(currentSelectedBlockPlace - glm::ivec3(0,1,0));
					if (b2 && canPlaceRedstoneOn(b2->get().x))
					{
						b->set(currentBlock, 0);
					}
				}
				else
				{
					b->set(currentBlock, 0);
				}
			}
		}
	}

	renderer.flush();

	ImGui::Begin("camera editor");

	ImGui::DragFloat2("camera", &renderer.currentCamera.position[0], 2, -3200, 3200);


	ImGui::DragInt3("block selector", &blockSelector[0], 1, 0);



	{
		auto uv1 = tilesAtlas.get(currentBlock, 0);
		ImGui::Image((void *)(intptr_t)tiles.id,
			{35,35}, {uv1.x, uv1.y}, {uv1.z, uv1.w});
	}

	ImGui::NewLine();
	{
		for (int mCount = 0; mCount < blocksCount; mCount++)
		{
			auto uv1 = tilesAtlas.get(mCount, 0);

			ImGui::PushID(mCount);
			if (ImGui::ImageButton((void *)(intptr_t)tiles.id,
				{35,35}, {uv1.x, uv1.y}, {uv1.z, uv1.w}))
			{
				currentBlock = mCount;
			}

			ImGui::PopID();

			if ((mCount + 1) % 5 != 0)
			{
				ImGui::SameLine();
			}

		}
	}

	if (input.buttons[pika::Button::Z].pressed()) { currentBlock--; }
	if (input.buttons[pika::Button::X].pressed()) { currentBlock++; }

	currentBlock = glm::clamp(currentBlock, 0, blocksCount-1);

	ImGui::NewLine();

	//ImGui::DragInt3("Cursor pos", &currentSelectedBlockDelete[0], 1);
	//currentSelectedBlock = glm::clamp(currentSelectedBlock, glm::ivec3{0}, map.size - glm::ivec3(1));

	ImGui::NewLine();


	ImGui::DragInt3("NewMapSize", &newMapSize[0]);

	if (ImGui::Button("Resize Map"))
	{
		Map newMap;
		newMap.init(newMapSize);

		for (int x = 0; x < map.size.x; x++)
			for (int y = 0; y < map.size.y; y++)
				for (int z = 0; z < map.size.z; z++)
				{

					auto b = map.getSafe({x,y,z});

					auto b2 = newMap.getSafe({x,y,z});

					if (b2)
					{
						*b2 = *b;
					}
				}

		map = std::move(newMap);
	}

	if (ImGui::Button("Replace floor"))
	{
		for (int x = 0; x < map.size.x; x++)
			for (int z = 0; z < map.size.z; z++)
			{

				auto b = map.getSafe({x,0,z});
				b->set(currentBlock, 0);
			}

	}

	ImGui::NewLine();


	loadedLevel.run(requestedInfo.requestedImguiIds);

	if (ImGui::Button("save map"))
	{
		if (requestedInfo.writeEntireFileBinary(loadedLevel.file, &map.size, sizeof(map.size)))
		{
			requestedInfo.appendFileBinary(loadedLevel.file, map.mapData.data(),
				sizeof(Block) * map.mapData.size());

			//toto log errors
		}
	}


	ImGui::NewLine();




	ImGui::End();



	return true;
}

void IsometricGameEditor::destruct(RequestedContainerInfo &requestedInfo)
{
	renderer.cleanup();

}
