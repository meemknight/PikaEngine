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

	char path[257] = {};

	glm::vec2 pos = {};

	int currentBlock = 0;
	bool flip = 0;
	
	Block *map;

	Block &getMapBlockUnsafe(int x, int y)
	{
		return map[x + y * mapSize.x];
	}

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		info.requestImguiFbo = true;
		info.requestImguiIds = 1;

		info.extensionsSuported = {".mario"};

		return info;
	}


	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{

		renderer.create();
		//gl2d::setErrorFuncCallback() //tood
		//pika::initShortcutApi();

		
		tiles = pika::gl2d::loadTextureWithPixelPadding(PIKA_RESOURCES_PATH "/mario/1985_tiles.png", requestedInfo, 8, true, false);
		if (tiles.id == 0) { return 0; }


		atlas = gl2d::TextureAtlasPadding(8, 10, 8*8, 8*10);


		map = new Block[mapSize.x * mapSize.y];
		Block d{27,0};
		memset(map, *(int *)(&d), mapSize.x * mapSize.y);

		if (commandLineArgument.size() != 0)
		{
			memcpy(path, commandLineArgument.data(), commandLineArgument.size());

			size_t s = 0;
			if (requestedInfo.getFileSizeBinary(commandLineArgument.to_string().c_str(), s))
			{
				if (s == mapSize.x * mapSize.y)
				{
					requestedInfo.readEntireFileBinary(commandLineArgument.to_string().c_str(), map, mapSize.x * mapSize.y);
				}
			}

		}

		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
		{
			glClear(GL_COLOR_BUFFER_BIT);
			gl2d::enableNecessaryGLFeatures();
			renderer.updateWindowMetrics(windowState.w, windowState.h);
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

			renderer.currentCamera.follow(pos, input.deltaTime * speed * 0.9f, 0.0001, 0.2, windowState.w, windowState.h);

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
					auto uv = getTileUV(atlas, b.type, b.flipped);

					renderer.renderRectangle({i, j, 1, 1}, {}, {}, tiles, uv);

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
				glm::vec2(viewRect.x + viewRect.z, viewRect.y + viewRect.w), glm::vec2(mousePos) / glm::vec2(windowState.w, windowState.h));

			if (blockPosition.x >= maxV.x || blockPosition.y >= maxV.y || blockPosition.x < minV.x || blockPosition.y < minV.y)
			{
				blockPosition = {-1,-1};
			}
			else
			{
			renderer.renderRectangle({blockPosition, 1, 1}, {0.9,0.9,0.9,0.9}, {}, {}, tiles, 
				getTileUV(atlas, currentBlock, flip));
			}

		}


		ImGui::Begin("Block picker");
		{
			bool collidable = true;
			bool nonCollidable = true;

			ImGui::Checkbox("Show Collidable Blocks", &collidable);
			ImGui::Checkbox("Show Non-Collidable Blocks", &nonCollidable);
			ImGui::Checkbox("Flip", &flip);
			ImGui::Text("MousePos: %d, %d", blockPosition.x, blockPosition.y);

			ImGui::InputText("Save file", path, sizeof(path));
			
			if (ImGui::Button("save"))
			{
				pika::memory::setGlobalAllocatorToStandard();
				sfs::writeEntireFile((void*)map, mapSize.x * mapSize.y, path);
				pika::memory::setGlobalAllocator(requestedInfo.mainAllocator);
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
					auto uv = getTileUV(atlas, mCount);

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
						if (isSolid(mCount))
						{
							auto uv = getTileUV(atlas, mCount);

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
						if (!isSolid(mCount))
						{
							auto uv = getTileUV(atlas, mCount);

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

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release