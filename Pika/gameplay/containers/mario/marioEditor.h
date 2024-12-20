#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include "marioCommon.h"
#include <safeSave/safeSave.h>
#include <engineLibraresSupport/engineGL2DSupport.h>

struct MarioEditor: public Container
{

	gl2d::Renderer2D renderer;
	gl2d::Texture tiles;
	gl2d::TextureAtlasPadding atlas;
	glm::ivec2 mapSize = {100, 100};

	glm::ivec2 mapSizeEditor = {100, 100};


	char path[257] = {};

	glm::vec2 pos = {};

	int currentBlock = 0;
	bool flip = 0;
	
	mario::Block *map;

	mario::Block &getMapBlockUnsafe(int x, int y)
	{
		return map[x + y * mapSize.x];
	}

	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		info.requestImguiFbo = true;
		info.requestImguiIds = 1;
		info.pushAnImguiIdForMe = true;
		
		info.extensionsSuported = {".mario"};

		return info;
	}


	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{

		renderer.create(requestedInfo.requestedFBO.fbo);
		//gl2d::setErrorFuncCallback() //tood
		//pika::initShortcutApi();

		
		tiles = pika::gl2d::loadTextureWithPixelPadding(PIKA_RESOURCES_PATH "/mario/1985_tiles.png", requestedInfo, 8, true, false);
		if (tiles.id == 0) { return 0; }


		atlas = gl2d::TextureAtlasPadding(8, 10, 8*8, 8*10);
		
		bool rez = mario::loadMap(requestedInfo, commandLineArgument.to_string(), &map, mapSize);
		mapSizeEditor = mapSize;

		if (rez)
		{
			pika::strlcpy(path, commandLineArgument.data(), commandLineArgument.size()+1);
			//path = commandLineArgument;
		}

		return rez;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
		{
			glClear(GL_COLOR_BUFFER_BIT);
			gl2d::enableNecessaryGLFeatures();
			renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
		}
		
		{
			float wheel = ImGui::GetIO().MouseWheel;

			//todo standard out

			if ((ImGui::GetIO().KeysData[ImGuiKey_LeftCtrl].Down || ImGui::GetIO().KeysData[ImGuiKey_RightCtrl].Down) && input.hasFocus)
			{
				renderer.currentCamera.zoom += wheel * 3;
			}

			renderer.currentCamera.zoom = std::min(renderer.currentCamera.zoom, 200.f);
			renderer.currentCamera.zoom = std::max(renderer.currentCamera.zoom, 10.f);

			glm::vec2 delta = {};

			if (input.hasFocus)
			{
				if (input.buttons[pika::Button::A].held())
				{
					delta.x -= 1;
				}
				if (input.buttons[pika::Button::D].held())
				{
					delta.x += 1;
				}
				if (input.buttons[pika::Button::W].held())
				{
					delta.y -= 1;
				}
				if (input.buttons[pika::Button::S].held())
				{
					delta.y += 1;
				}
			}

			float speed = 10;

			delta *= input.deltaTime * speed;

			pos += delta;

			//todo update gl2d this function

			renderer.currentCamera.follow(pos, input.deltaTime * speed * 0.9f, 0.0001, 0.2, windowState.windowW, windowState.windowH);

		}
		auto viewRect = renderer.getViewRect();

		glm::ivec2 minV;
		glm::ivec2 maxV;
		//render
		{
			minV = {viewRect.x-1, viewRect.y-1};
			maxV = minV + glm::ivec2{viewRect.z+2, viewRect.w+2};
			minV = glm::max(minV, {0,0});
			maxV = glm::min(maxV, mapSize);
		
		
			for (int j = minV.y; j < maxV.y; j++)
				for (int i = minV.x; i < maxV.x; i++)
				{
					auto b = getMapBlockUnsafe(i, j);
					auto uv = mario::getTileUV(atlas, b.type, b.flipped);

					renderer.renderRectangle({i, j, 1, 1}, tiles, Colors_White, {}, 0, uv);

				}
		}

		//mouse pos
		glm::ivec2 blockPosition;
		{
			glm::ivec2 mousePos(input.mouseX, input.mouseY);

			auto lerp = [](auto a, auto b, auto c)
			{
				return a * (1.f - c) + b * c;
			};

			blockPosition = lerp(glm::vec2(viewRect.x, viewRect.y),
				glm::vec2(viewRect.x + viewRect.z, viewRect.y + viewRect.w), glm::vec2(mousePos) / glm::vec2(windowState.windowW, windowState.windowH));

			if (blockPosition.x >= maxV.x || blockPosition.y >= maxV.y || blockPosition.x < minV.x || blockPosition.y < minV.y)
			{
				blockPosition = {-1,-1};
			}
			else
			{
				renderer.renderRectangle({blockPosition, 1, 1}, tiles, {0.9,0.9,0.9,0.9}, {}, 0,
				mario::getTileUV(atlas, currentBlock, flip));
			}

		}

		if (input.buttons[pika::Button::F].released())
		{
			flip = !flip;
		}

		ImGui::Begin("Block picker");
		{
			bool collidable = true;
			bool nonCollidable = true;

			ImGui::Checkbox("Show Collidable Blocks", &collidable);
			ImGui::Checkbox("Show Non-Collidable Blocks", &nonCollidable);
			ImGui::Checkbox("Flip", &flip);
			ImGui::Text("MousePos: %d, %d", blockPosition.x, blockPosition.y);

			ImGui::InputInt2("Map size", &mapSizeEditor[0]); ImGui::SameLine();

			if (ImGui::Button("Set size"))
			{
				mario::Block *newMap = 0;
				newMap = new mario::Block[mapSizeEditor.x * mapSizeEditor.y];
				mario::Block d{27,0};
				memset(newMap, *(int *)(&d), mapSizeEditor.x *mapSizeEditor.y);

				for (int y = 0; y < std::min(mapSize.y, mapSizeEditor.y); y++)
				{
					for (int x = 0; x < std::min(mapSize.x, mapSizeEditor.x); x++)
					{
						newMap[x + y * mapSizeEditor.x] = map[x + y * mapSize.x];
					}
				}
				
				delete[] map;
				map = newMap;
				mapSize = mapSizeEditor;
			}


			ImGui::InputText("Save file", path, sizeof(path));
			
			if (ImGui::Button("save"))
			{
				requestedInfo.writeEntireFileBinary(path, &mapSize, sizeof(mapSize));
				requestedInfo.appendFileBinary(path, (void *)map, mapSize.x *mapSize.y);
			}

			ImGui::Separator();

			unsigned short mCount = 0;
			ImGui::BeginChild("Block Selector");
			bool inImgui = ImGui::IsWindowHovered();

			if (collidable && nonCollidable)
			{
				unsigned short localCount = 0;
				while (mCount < 8*10)
				{
					auto uv = mario::getTileUV(atlas, mCount);

					ImGui::PushID(mCount);
					if (ImGui::ImageButton((void *)(intptr_t)tiles.id,
						{35,35}, {uv.x, uv.y}, {uv.z, uv.w}))
					{
						currentBlock = mCount;
					}

					ImGui::PopID();

					if (localCount % 10 != 0)
					{
						ImGui::SameLine();
					}
					localCount++;

					mCount++;
				}
			}
			else
			{
				if (collidable && !nonCollidable)
				{
					unsigned short localCount = 0;
					while (mCount < 8 * 10)
					{
						if (mario::isSolid(mCount))
						{
							auto uv = mario::getTileUV(atlas, mCount);

							ImGui::PushID(mCount);
							if (ImGui::ImageButton((void *)(intptr_t)tiles.id,
								{35,35}, {uv.x, uv.y}, {uv.z, uv.w}));
							{
								currentBlock = mCount;
							}
							ImGui::PopID();

							if (localCount % 10 != 0)
							{
								ImGui::SameLine();
							}
							localCount++;

						}
						mCount++;
					}
				}
				else if (!collidable && nonCollidable)
				{
					unsigned short localCount = 0;
					while (mCount < 8*10)
					{
						if (!mario::isSolid(mCount))
						{
							auto uv = mario::getTileUV(atlas, mCount);

							ImGui::PushID(mCount);
							if (ImGui::ImageButton((void *)(intptr_t)tiles.id,
								{35,35}, {uv.x, uv.y}, {uv.z, uv.w}));
							{
								currentBlock = mCount;
							}
							ImGui::PopID();

							if (localCount % 10 != 0)
							{
								ImGui::SameLine();
							}
							localCount++;

						}
						mCount++;
					}
				}
			}
			ImGui::EndChild();



		}
		ImGui::End();
	

		if (input.hasFocus && input.lMouse.held() && blockPosition.x >= 0)
		{
			if (input.buttons[pika::Button::LeftCtrl].held())
			{

				currentBlock = getMapBlockUnsafe(blockPosition.x, blockPosition.y).type;
				flip = getMapBlockUnsafe(blockPosition.x, blockPosition.y).flipped;

			}
			else
			{
				getMapBlockUnsafe(blockPosition.x, blockPosition.y).type = currentBlock;
				getMapBlockUnsafe(blockPosition.x, blockPosition.y).flipped = flip;

			}

		}


		renderer.flush();
	
		return true;
	}

	void destruct(RequestedContainerInfo &requestedInfo) override
	{
		tiles.cleanup();
		renderer.cleanup();
		delete[] map;
	}

};
