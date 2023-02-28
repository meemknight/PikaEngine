#pragma once

#include <gl2d/gl2d.h>
#include <gl3d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <imfilebrowser.h>
#include <engineLibraresSupport/engineGL3DSupport.h>
#include "blocks.h"
#include <stringManipulation/stringManipulation.h>

struct McDungeonsEditor: public Container
{


	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(1000); //todo option to use global allocator

		info.extensionsSuported = {".mcDungeons"};

		info.requestImguiFbo = true;
		info.requestImguiIds = 1;

		return info;
	}

	gl2d::Renderer2D renderer2d;
	gl3d::Renderer3D renderer;
	gl3d::Model model;
	gl3d::Entity entity;
	bool first = 1;
	GLuint blocksTexture; //todo add genderer.getgputexture
	int currentBlock = 0;

	pika::gl3d::General3DEditor editor;
	pika::pikaImgui::FileSelector loadedLevel;

	glm::vec3 worldSize = {150,40,150};
	unsigned char worldData[150][40][150] = {};

	unsigned char &getBlockUnsafe(int x, int y, int z)
	{
		return worldData[x][y][z];
	}

	bool isInRange(int x, int y, int z)
	{
		if (x < 0 || y < 0 || z < 0 || x >= worldSize.x || y >= worldSize.y || z >= worldSize.z)
			return 0;

		return 1;
	}

	unsigned char stub = 0;
	unsigned char &getBlockSafe(int x, int y, int z)
	{
		if (!isInRange(x,y,z))
		{
			stub = 0;
			return stub;
		}
		return getBlockUnsafe(x, y, z);
	}

	bool rayMarch(glm::vec3 pos, glm::vec3 dir, float distance, glm::vec3 *hitPos, glm::vec3 *prevHitPos)
	{
		glm::vec3 march = pos;
		const float increment = 0.1;
		dir = glm::normalize(dir) * increment;
		march += dir;

		glm::vec3 prev = march;

		for (int i = 1; i < distance / increment; i++)
		{
			if (getBlockSafe(march.x, march.y, march.z) != 0)
			{
				if (hitPos) { *hitPos = march; }
				if (prevHitPos) { *prevHitPos = prev; }
				return 1;
			}
			prev = march;
			march += dir;
		}

		return 0;
	}

	void populateWorld()
	{
		for (int x = 0; x < worldSize.x; x++)
			for (int z = 0; z < worldSize.z; z++)
			{
				for(int y=0; y<10; y++)
				{
					getBlockUnsafe(x, y, z) = BlockTypes::stone;
				}
				for (int y = 10; y < 12; y++)
				{
					getBlockUnsafe(x, y, z) = BlockTypes::dirt;
				}

				getBlockUnsafe(x, 12, z) = BlockTypes::grassBlock;

			}

		for (int x = 2; x < 8; x++)
			for (int z = 2; z < 8; z++)
			{
				getBlockUnsafe(x, 4, z) = BlockTypes::stone;
			}

		//getBlockUnsafe(4, 5, 4) = BlockTypes::glowStone;
		//getBlockUnsafe(5, 5, 4) = BlockTypes::redstone_ore;
		//
		//getBlockUnsafe(7, 6, 4) = BlockTypes::crafting_table;
		//getBlockUnsafe(9, 6, 4) = BlockTypes::furnace_on;
		//getBlockUnsafe(3, 6, 7) = BlockTypes::furnace_off;
		//getBlockUnsafe(2, 5, 9) = BlockTypes::iron_block;

		getBlockUnsafe(0, 0, 0) = BlockTypes::gold_block;

	}

	gl3d::Model createWorld(gl3d::Renderer3D &renderer, gl3d::Material material)
	{

		auto &pointLights = renderer.internal.pointLightIndexes;
		while(!pointLights.empty())
		{
			::gl3d::PointLight light;
			light.id_ = pointLights.back();
			renderer.detletePointLight(light);
		}

		auto &spoLights = renderer.internal.spotLightIndexes;
		while (!spoLights.empty())
		{
			::gl3d::SpotLight light;
			light.id_ = spoLights.back();
			renderer.deleteSpotLight(light);
		}


	#pragma region data

		float uv = 1;

		std::vector<float> vertexes;
		std::vector<unsigned int> indices;

		std::vector<unsigned int> ind = {0,   1,  2,  0,  2,  3};

		std::vector<float> topVer = {
				-1.0f, +1.0f, +1.0f, // 0
				+0.0f, +1.0f, +0.0f, // Normal
				0, 0,				 //uv

				+1.0f, +1.0f, +1.0f, // 1
				+0.0f, +1.0f, +0.0f, // Normal
				1 * uv, 0,				 //uv

				+1.0f, +1.0f, -1.0f, // 2
				+0.0f, +1.0f, +0.0f, // Normal
				1 * uv, 1 * uv,				 //uv

				-1.0f, +1.0f, -1.0f, // 3
				+0.0f, +1.0f, +0.0f, // Normal
				0, 1 * uv,				 //uv
		};

		std::vector<float> backVer = {
			-1.0f, +1.0f, -1.0f, // 4
				 0.0f, +0.0f, -1.0f, // Normal
				 0, 1 * uv,				 //uv

				+1.0f, +1.0f, -1.0f, // 5
				 0.0f, +0.0f, -1.0f, // Normal
				 1 * uv, 1 * uv,				 //uv

				 +1.0f, -1.0f, -1.0f, // 6
				 0.0f, +0.0f, -1.0f, // Normal
				 1 * uv, 0,				 //uv

				-1.0f, -1.0f, -1.0f, // 7
				 0.0f, +0.0f, -1.0f, // Normal
				 0, 0,				 //uv
		};

		std::vector<float> rightVer = {
			+1.0f, +1.0f, -1.0f, // 8
				+1.0f, +0.0f, +0.0f, // Normal
				1, 1,				 //uv

				+1.0f, +1.0f, +1.0f, // 9
				+1.0f, +0.0f, +0.0f, // Normal
				0, 1,				 //uv

				+1.0f, -1.0f, +1.0f, // 10
				+1.0f, +0.0f, +0.0f, // Normal
				0, 0,				 //uv

				+1.0f, -1.0f, -1.0f, // 11
				+1.0f, +0.0f, +0.0f, // Normal
				1, 0,				 //uv
		};

		std::vector<float> leftVer = {
		-1.0f, +1.0f, +1.0f, // 12
				-1.0f, +0.0f, +0.0f, // Normal
				1 * uv, 1 * uv,				 //uv

				-1.0f, +1.0f, -1.0f, // 13
				-1.0f, +0.0f, +0.0f, // Normal
				0 * uv, 1,				 //uv

				-1.0f, -1.0f, -1.0f, // 14
				-1.0f, +0.0f, +0.0f, // Normal
				0, 0,				 //uv

				-1.0f, -1.0f, +1.0f, // 15
				-1.0f, +0.0f, +0.0f, // Normal
				1, 0 * uv,				 //uv
		};

		std::vector<float> frontVer = {
			+1.0f, +1.0f, +1.0f, // 16
		+0.0f, +0.0f, +1.0f, // Normal
		1 * uv, 1 * uv,				 //uv

		-1.0f, +1.0f, +1.0f, // 17
		+0.0f, +0.0f, +1.0f, // Normal
		0, 1 * uv,				 //uv

		-1.0f, -1.0f, +1.0f, // 18
		+0.0f, +0.0f, +1.0f, // Normal
		0, 0,				 //uv

		+1.0f, -1.0f, +1.0f, // 19
		+0.0f, +0.0f, +1.0f, // Normal
		1 * uv, 0,				 //uv
		};

		std::vector<float> bottomVer = {
			-1.0f, -1.0f, -1.0f, // 3
		+0.0f, -1.0f, +0.0f, // Normal
		0, 1 * uv,				 //uv

		+1.0f, -1.0f, -1.0f, // 2
		+0.0f, -1.0f, +0.0f, // Normal
		1 * uv, 1 * uv,				 //uv

		+1.0f, -1.0f, +1.0f, // 1
		+0.0f, -1.0f, +0.0f, // Normal
		1 * uv, 0,				 //uv

		-1.0f, -1.0f, +1.0f, // 0
		+0.0f, -1.0f, +0.0f, // Normal
		0, 0,				 //uv

		};

		auto addFace = [&](glm::vec3 pos, std::vector<float> &ver, glm::vec2 atlas)
		{

			unsigned int currentIndexPadding = vertexes.size() / 8;
			for (auto i : ind) { indices.push_back(i + currentIndexPadding); }

			for (auto v = 0; v < ver.size(); v++)
			{
				if (v % 8 == 0)
				{
					vertexes.push_back(ver[v] * 0.5 + pos.x);
				}
				else if (v % 8 == 1)
				{
					vertexes.push_back(ver[v] * 0.5 + pos.y);
				}
				else if (v % 8 == 2)
				{
					vertexes.push_back(ver[v] * 0.5 + pos.z);
				}
				else if (v % 8 == 6)
				{
					vertexes.push_back(ver[v] / 16.f + atlas.x / 16.f);
				}
				else if (v % 8 == 7)
				{
					vertexes.push_back(ver[v] / 16.f + atlas.y / 16.f);
				}
				else
				{
					vertexes.push_back(ver[v]);
				}
			}
		};

		auto addTop = [&](glm::vec3 pos, int blockType)
		{
			return addFace(pos, topVer, getAtlasTop(blockType));
		};

		auto addBottom = [&](glm::vec3 pos, int blockType)
		{
			return addFace(pos, bottomVer, getAtlasBottom(blockType));
		};

		auto addLeft = [&](glm::vec3 pos, int blockType)
		{
			return addFace(pos, leftVer, getAtlasLeft(blockType));
		};

		auto addRight = [&](glm::vec3 pos, int blockType)
		{
			return addFace(pos, rightVer, getAtlasRight(blockType));
		};

		auto addFront = [&](glm::vec3 pos, int blockType)
		{
			return addFace(pos, frontVer, getAtlasFront(blockType));
		};

		auto addBack = [&](glm::vec3 pos, int blockType)
		{
			return addFace(pos, backVer, getAtlasBack(blockType));
		};
	#pragma endregion

		auto addCube = [&](glm::vec3 pos, int blockType)
		{
			addTop(pos, blockType);
			addBottom(pos, blockType);
			addLeft(pos, blockType);
			addRight(pos, blockType);
			addFront(pos, blockType);
			addBack(pos, blockType);
		};

		auto isSolid = [](unsigned char b) { return b != 0 && b != BlockTypes::glass && b != BlockTypes::glowStone
			&& b != BlockTypes::furnace_on; };

		for (int x = 0; x < worldSize.x; x++)
			for (int y = 0; y < worldSize.y; y++)
				for (int z = 0; z < worldSize.z; z++)
				{
					auto b = getBlockUnsafe(x, y, z);

					if (b)
					{
						if (y == worldSize.y - 1 || !isSolid(getBlockUnsafe(x, y + 1, z)))
						{
							addTop({x,y,z}, b);
						}

						if (y == 0 || !isSolid(getBlockUnsafe(x, y - 1, z)))
						{
							addBottom({x,y,z}, b);
						}
	
						if (x == worldSize.x - 1 || !isSolid(getBlockUnsafe(x + 1, y, z)))
						{
							addRight({x,y,z}, b);
						}

						if (x == 0 || !isSolid(getBlockUnsafe(x - 1, y, z)))
						{
							addLeft({x,y,z}, b);
						}

						if (z == worldSize.z - 1 || !isSolid(getBlockUnsafe(x, y, z + 1)))
						{
							addFront({x,y,z}, b);

						}

						if (z == 0 || !isSolid(getBlockUnsafe(x, y, z - 1)))
						{
							addBack({x,y,z}, b);
						}

						if (b == BlockTypes::glowStone)
						{
							renderer.createPointLight({x,y,z}, glm::vec3(0.845f, 0.812f, 0.381f)*1.5f, 16.f, 2.f);
						}else 
						if (b == BlockTypes::furnace_on)
						{
							renderer.createSpotLight({x,y,z}, glm::radians(90.f), glm::vec3(0,0,1), 16, 1.5, glm::vec3(1, 0.5f, 0.211f) * 2.f);
						}

					}
				}

		return renderer.createModelFromData(material, "world",
			vertexes.size(), vertexes.data(), indices.size(),
			indices.data());
	}

	gl3d::Material mat;

	std::string currentFile = {};

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{

		renderer2d.create();

		//todo close function for containers
		//todo check out why mouse don't work in outside window

		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;
		
		renderer.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", requestedInfo.requestedFBO.fbo);
		renderer.colorCorrectionTexture() = renderer.loadColorLookupTextureFromFile(PIKA_RESOURCES_PATH "/mcDungeons/lut.png");

		//renderer.skyBox = renderer.loadSkyBox(names);
		//renderer.skyBox.color = {0.2,0.3,0.8};
		renderer.skyBox = renderer.atmosfericScattering({0,0.7,0.3}, {0.2,0.2,0.5}, {0.6,0.2,0.1}, {},
			false, 10);
		renderer.frustumCulling = false;
		renderer.skyBox.color = glm::vec3(0.659f, 0.698f, 0.723f);

		renderer.createDirectionalLight(glm::vec3(0.575, -0.686, 0.445), glm::vec3(0.155f, 0.893f, 1.000f), 2);


		//helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "helmet/helmet.obj");
		//model = renderer.loadModel(PIKA_RESOURCES_PATH "rave.glb", 0.5);
		auto defaultMat = renderer.loadMaterial(PIKA_RESOURCES_PATH "materials/mcSprites/mc.mtl", gl3d::TextureLoadQuality::leastPossible); //todo quality settings
		if (defaultMat.empty()) { return 0; }
		mat = defaultMat[0];


		auto textures = renderer.getMaterialTextures(defaultMat[0]);
		blocksTexture = renderer.getTextureOpenglId(textures.albedoTexture);


		textures.pbrTexture.RMA_loadedTextures &= 0b110;
		renderer.setMaterialTextures(defaultMat[0], textures);

		
		loadedLevel.setInfo("Level", PIKA_RESOURCES_PATH, {".mcDungeons"});

		bool created = 0;
		if (commandLineArgument.size() > 0)
		{
			//editor.loadFromFile(renderer, commandLineArgument.to_string(), requestedInfo);
			size_t s = 0;
			pika::strlcpy(loadedLevel.file, commandLineArgument.to_string(), sizeof(loadedLevel.file));

			if (requestedInfo.getFileSizeBinary(commandLineArgument.to_string().c_str(), s))
			{

				if (s == worldSize.x * worldSize.y * worldSize.z)
				{
					created = 1;
					requestedInfo.readEntireFileBinary(commandLineArgument.to_string().c_str(), worldData, sizeof(worldData));
				}
			}
		}

		if (!created)
		{
			populateWorld();
		}


		model = createWorld(renderer, defaultMat[0]);
		entity = renderer.createEntity(model);

		renderer.camera.farPlane = 200;
		renderer.directionalShadows.frustumSplits[0] = 0.06;
		renderer.directionalShadows.frustumSplits[1] = 0.110;
		renderer.directionalShadows.frustumSplits[2] = 0.200;

		
		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{

		renderer2d.updateWindowMetrics(windowState.w, windowState.h);

		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_MULTISAMPLE);


		renderer.updateWindowMetrics(windowState.w, windowState.h);
		renderer.camera.aspectRatio = (float)windowState.w / windowState.h; //todo do this in update


		editor.update(requestedInfo.requestedImguiIds, renderer, input, 4, requestedInfo, {windowState.w,windowState.h});
	
		bool shouldRecreate = 0;

	#pragma region blocks
		{
			glm::vec3 block = {};
			glm::vec3 prev = {};

			if (input.buttons[pika::Button::LeftCtrl].held())
			{
				if (input.rMouse.pressed() || input.lMouse.pressed())
				{
					auto cameraRayPos = renderer.camera.position;
					cameraRayPos.y += 0.5;
					cameraRayPos.x += 0.5;
					cameraRayPos.z += 0.5;
					if (rayMarch(cameraRayPos, renderer.camera.viewDirection, 10, &block, nullptr))
					{
						currentBlock = getBlockSafe(block.x, block.y, block.z);
					}
				}
			}else
			if (input.lMouse.pressed())
			{
				auto cameraRayPos = renderer.camera.position;
				cameraRayPos.y += 0.5;
				cameraRayPos.x += 0.5;
				cameraRayPos.z += 0.5;
				if (rayMarch(cameraRayPos, renderer.camera.viewDirection, 10, &block, nullptr))
				{
					getBlockSafe(block.x, block.y, block.z) = 0;
					shouldRecreate = 1;
				}
			}else 
			if (input.rMouse.pressed() && currentBlock != 0)
			{
				auto cameraRayPos = renderer.camera.position;
				cameraRayPos.y += 0.5;
				cameraRayPos.x += 0.5;
				cameraRayPos.z += 0.5;
				if (rayMarch(cameraRayPos, renderer.camera.viewDirection, 10, nullptr, &block))
				{
					getBlockSafe(block.x, block.y, block.z) = currentBlock;
					shouldRecreate = 1;
				}
			}

		}
	#pragma endregion


		ImGui::PushID(requestedInfo.requestedImguiIds);

		if (input.buttons[pika::Button::Z].released())
		{
			currentBlock--;
		}

		if (input.buttons[pika::Button::X].released())
		{
			currentBlock++;
		}

		currentBlock = std::max(0, currentBlock);
		currentBlock = std::min(BlocksCount-1, currentBlock);

		if (ImGui::Begin("General3DEditor"))
		{
			if (ImGui::Button("recreate world"))
			{
				shouldRecreate = 1;
			}

			ImGui::DragFloat3("camera pos", &renderer.camera.position[0]);

			ImGui::NewLine();

			{
				auto uv1 = getAtlasFront(currentBlock);
				glm::vec4 uv{
					0.f / 16.f + uv1.x / 16.f,
					1.f / 16.f + uv1.y / 16.f,
					1.f / 16.f + uv1.x / 16.f,
					0.f / 16.f + uv1.y / 16.f
				};
				ImGui::Image((void *)(intptr_t)blocksTexture,
					{35,35}, {uv.x, uv.y}, {uv.z, uv.w});
			}

			ImGui::NewLine();
			{
				for(int mCount =0; mCount <BlockTypes::BlocksCount; mCount++)
				{
					auto uv1 = getAtlasFront(mCount);
					glm::vec4 uv{
						0.f / 16.f + uv1.x / 16.f,
						1.f / 16.f + uv1.y / 16.f,
						1.f / 16.f + uv1.x / 16.f,
						0.f / 16.f + uv1.y / 16.f
					};

				

					ImGui::PushID(mCount);
					if (ImGui::ImageButton((void *)(intptr_t)blocksTexture,
						{35,35}, {uv.x, uv.y}, {uv.z, uv.w}))
					{
						currentBlock = mCount;
					}

					ImGui::PopID();

					if ((mCount+1) % 5 != 0)
					{
						ImGui::SameLine();
					}

				}
			}

			ImGui::NewLine();

			loadedLevel.run(requestedInfo.requestedImguiIds);

			if (ImGui::Button("save map"))
			{
				if (!requestedInfo.writeEntireFileBinary(loadedLevel.file, worldData, sizeof(worldData)))
				{
					//toto log errors
					//requestedInfo.con
				}
			}

		}


		ImGui::End();

		ImGui::PopID();

		if (shouldRecreate)
		{
			renderer.deleteModel(model);
			model = createWorld(renderer, mat);
			renderer.setEntityModel(entity, model);
		}


		renderer.render(input.deltaTime);
		glDisable(GL_DEPTH_TEST);


		renderer2d.renderRectangle({windowState.w / 2 - 5, windowState.h / 2 - 5,10,10}, Colors_Orange);


		renderer2d.flush();

		//requestedInfo.consoleWrite(
		//	(std::to_string(requestedInfo.internal.windowPosX) + " " + std::to_string(requestedInfo.internal.windowPosY) + "\n").c_str()
		//);

		return true;
	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release