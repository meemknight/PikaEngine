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


struct McDungeonsEditor: public Container
{


	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(1000); //todo option to use global allocator

		info.extensionsSuported = {".gl3d"};

		info.requestImguiFbo = true;
		info.requestImguiIds = 1;

		return info;
	}


	gl3d::Renderer3D renderer;
	gl3d::Model model;
	gl3d::Entity entity;
	bool first = 1;


	pika::gl3d::General3DEditor editor;

	unsigned char worldData[100][10][100] = {};

	unsigned char &getBlockUnsafe(int x, int y, int z)
	{
		return worldData[x][y][z];
	}

		 
	gl3d::Model createWorld(gl3d::Renderer3D &renderer, gl3d::Material material)
	{

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


		for (int x = 0; x < 10; x++)
			for (int z = 0; z < 10; z++)
			{
				getBlockUnsafe(x, 0, z) = BlockTypes::grassBlock;
			}

		for (int x = 2; x < 8; x++)
			for (int z = 2; z < 8; z++)
			{
				getBlockUnsafe(x, 4, z) = BlockTypes::stone;
			}

		getBlockUnsafe(4, 5, 4) = BlockTypes::gold_block;
		getBlockUnsafe(5, 5, 4) = BlockTypes::gold_block;


		glm::vec3 size = {100,10,100};

		auto isSolid = [](unsigned char b) { return b != 0; };

		for (int x = 0; x < size.x; x++)
			for (int y = 0; y < size.y; y++)
				for (int z = 0; z < size.z; z++)
				{
					auto b = getBlockUnsafe(x, y, z);

					if (b)
					{
						if (y == size.y - 1 || !isSolid(getBlockUnsafe(x, y + 1, z)))
						{
							addTop({x,y,z}, b);
						}

						if (y == 0 || !isSolid(getBlockUnsafe(x, y - 1, z)))
						{
							addBottom({x,y,z}, b);
						}
	
						if (x == size.x - 1 || !isSolid(getBlockUnsafe(x + 1, y, z)))
						{
							addRight({x,y,z}, b);
						}

						if (x == 0 || !isSolid(getBlockUnsafe(x - 1, y, z)))
						{
							addLeft({x,y,z}, b);
						}

						if (z == size.z - 1 || !isSolid(getBlockUnsafe(x, y, z + 1)))
						{
							addFront({x,y,z}, b);

						}

						if (z == 0 || !isSolid(getBlockUnsafe(x, y, z - 1)))
						{
							addBack({x,y,z}, b);
						}

					}
				}


		return renderer.createModelFromData(material, "world",
			vertexes.size(), vertexes.data(), indices.size(),
			indices.data());
	}

	gl3d::Material mat;

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		//todo close function for containers
		//todo check out why mouse don't work in outside window

		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;
		
		renderer.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", requestedInfo.requestedFBO.fbo);
		
		//renderer.skyBox = renderer.loadSkyBox(names);
		//renderer.skyBox.color = {0.2,0.3,0.8};
		renderer.skyBox = renderer.atmosfericScattering({0,1,0}, {0.2,0.2,0.5}, {0.6,0.2,0.1}, {},
			false, 10);

		//helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "helmet/helmet.obj");
		//model = renderer.loadModel(PIKA_RESOURCES_PATH "rave.glb", 0.5);
		auto defaultMat = renderer.loadMaterial(PIKA_RESOURCES_PATH "materials/mcSprites/mc.mtl", gl3d::TextureLoadQuality::leastPossible); //todo quality settings
		if (defaultMat.empty()) { return 0; }
		mat = defaultMat[0];


		auto textures = renderer.getMaterialTextures(defaultMat[0]);



		textures.pbrTexture.RMA_loadedTextures &= 0b110;
		renderer.setMaterialTextures(defaultMat[0], textures);

		model = createWorld(renderer, defaultMat[0]);

		
		gl3d::Transform t;
		t.position = {0, -1, -4};
		//t.rotation = {1.5, 0 , 0};
		
		entity = renderer.createEntity(model, t);

		if (commandLineArgument.size() > 0)
		{
			editor.loadFromFile(renderer, commandLineArgument.to_string(), requestedInfo);
		}


		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{

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


		editor.update(requestedInfo.requestedImguiIds, renderer, input, 4, requestedInfo);

		ImGui::PushID(requestedInfo.requestedImguiIds);

		if (ImGui::Begin("General3DEditor"))
		{
			if (ImGui::Button("recreate world"))
			{
				renderer.deleteModel(model);
				model = createWorld(renderer, mat);
				renderer.setEntityModel(entity, model);
			}

		}


		ImGui::End();

		ImGui::PopID();


		renderer.render(input.deltaTime);
		glDisable(GL_DEPTH_TEST);


		return true;
	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release