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

struct McDungeonsGameplay: public Container
{


	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(1000); //todo option to use global allocator

		info.requestImguiFbo = true;
		info.requestImguiIds = 1;

		return info;
	}

	struct PhysicsComponent
	{
		glm::vec2 size = glm::vec2(0.3f, 0.3f);
		glm::vec2 position = {glm::vec2(0,0)};

		glm::vec2 lastPos{};

		glm::vec2 velocity = {};

		bool movingRight = 0;
		bool grounded = 0;

		float rotation = 0;
		float desiredRotation = 0;

		void updateMove() { lastPos = position; }
	};

	void resolveConstrains(PhysicsComponent &player);

	void checkCollisionBrute(PhysicsComponent &player, glm::vec2 &pos, glm::vec2 lastPos
		, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch);

	glm::vec2 performCollision(PhysicsComponent &player, glm::vec2 pos, glm::vec2 size,
		glm::vec2 delta, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch);


	struct Zombie
	{
		PhysicsComponent physics;
		gl3d::Entity entity;

		Zombie() {};
		Zombie(int x, int z) { physics.position = {x,z}; physics.lastPos = {x,z}; };
	};

	std::vector<Zombie> enemies;


	gl3d::Entity player;

	enum Animations
	{
		attack,
		idle,
		run,
		run2,
		zombieAttack,
		zombieIdle,
		zombieRun,
	};


	gl2d::Renderer2D renderer2d;
	gl3d::Renderer3D renderer;
	gl3d::Model model;
	gl3d::Entity entity;
	GLuint blocksTexture;

	gl3d::Entity sword;

	PhysicsComponent playerPhysics;
	

	pika::gl3d::General3DEditor editor;

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
		playerPhysics.position = {21,32};
		playerPhysics.lastPos = {21,32};

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

		bool created = 0;
		size_t s = 0;
		if (requestedInfo.getFileSizeBinary(PIKA_RESOURCES_PATH "/mcDungeons/map1.mcDungeons", s))
		{

			if (s == worldSize.x * worldSize.y * worldSize.z)
			{
				created = 1;
				requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "/mcDungeons/map1.mcDungeons", worldData, sizeof(worldData));
			}
		}

		if (!created)
		{
			return 0;
		}


		model = createWorld(renderer, defaultMat[0]);
		entity = renderer.createEntity(model);

		auto zombieMat = renderer.loadMaterial(PIKA_RESOURCES_PATH "mcDungeons/zombie.mtl", 0);
		if (zombieMat.size() != 1) { return false; }

		auto playerModel = renderer.loadModel(PIKA_RESOURCES_PATH "mcDungeons/steve.glb", 0, 1);
		//auto mat = renderer.loadMaterial(PIKA_RESOURCES_PATH "mcDungeons/steve.mtl", 0);
		//if (mat.size() < 1) { return false; }
		player = renderer.createEntity(playerModel, gl3d::Transform{glm::vec3{22,13,32}}, false);
		//{
		//	int count = renderer.getEntityMeshesCount(player);
		//	if (!count)return 0;
		//	for (int i = 0; i < count; i++)
		//	{
		//		renderer.setEntityMeshMaterial(player, i, mat[0]);
		//	}
		//}
		renderer.setEntityAnimate(player, true);
		renderer.setEntityAnimationIndex(player, Animations::idle);

		auto swordModel = renderer.loadModel(PIKA_RESOURCES_PATH "/mcDungeons/minecraft_sword.glb", gl3d::TextureLoadQuality::maxQuality, 0.1);
		sword = renderer.createEntity(swordModel, {}, false);

		//enemies
		{
			enemies.push_back(Zombie(22, 33));
			

			for (auto &i : enemies)
			{
				i.entity = renderer.createEntity(playerModel, gl3d::Transform{glm::vec3{i.physics.position.x, 13, i.physics.position.y}}, false);
			
				int count = renderer.getEntityMeshesCount(i.entity);
				if (!count)return 0;
				for (int j = 0; j < count; j++)
				{
					renderer.setEntityMeshMaterial(i.entity, j, zombieMat[0]);
				}
				
				renderer.setEntityAnimate(i.entity, true);
				renderer.setEntityAnimationIndex(i.entity, Animations::zombieIdle);
			}


		}


		renderer.camera.farPlane = 200;
		renderer.directionalShadows.frustumSplits[0] = 0.06;
		renderer.directionalShadows.frustumSplits[1] = 0.110;
		renderer.directionalShadows.frustumSplits[2] = 0.200;
		renderer.bloomHighQualityDownSample() = true;
		renderer.bloomHighQualityUpSample() = true;
		renderer.setSSAOExponent(6.f);
		renderer.setExposure(1.5f);
		renderer.frustumCulling = false;
		
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


	
		bool shouldRecreate = 0;


		ImGui::PushID(requestedInfo.requestedImguiIds);


		if (ImGui::Begin("General3DEditor"))
		{
			if (ImGui::Button("recreate world"))
			{
				shouldRecreate = 1;
			}

			ImGui::DragFloat3("camera pos", &renderer.camera.position[0]);

		}


		ImGui::End();

		ImGui::PopID();

		if (shouldRecreate)
		{
			renderer.deleteModel(model);
			model = createWorld(renderer, mat);
			renderer.setEntityModel(entity, model);
		}


	#pragma region gameplay

	#pragma region player
		{
			
		#pragma region input
			
			{
				glm::vec2 dir = {};

				if (input.buttons[pika::Button::A].held() || input.buttons[pika::Button::Left].held())
				{
					dir -= glm::vec2(1,1);
				}
				if (input.buttons[pika::Button::D].held() || input.buttons[pika::Button::Right].held())
				{
					dir += glm::vec2(1,1);
				}
				if (input.buttons[pika::Button::W].held() || input.buttons[pika::Button::Up].held())
				{
					dir += glm::vec2(1,-1);
				}
				if (input.buttons[pika::Button::S].held() || input.buttons[pika::Button::Down].held())
				{
					dir -= glm::vec2(1, -1);
				}

				if (dir.x != 0 || dir.y != 0)
				{
					dir = glm::normalize(dir);
					
					auto lerp = [&](float x, float y, float a)
					{
						a = std::min(a, 1.f);
						a = std::max(a, 0.f);
						return x * (1 - a) + y * (a);
					};
					
					playerPhysics.desiredRotation = std::atan2(dir.x, dir.y);
					renderer.setEntityAnimationIndex(player, Animations::run);
				}
				else
				{
					renderer.setEntityAnimationIndex(player, Animations::idle);
				}

				float speed = 3;
				playerPhysics.position += dir * input.deltaTime * speed;

			}

		#pragma endregion

		#pragma region rotate
			{
				const float pi2 = 3.1415926f * 2.f;

				if (playerPhysics.desiredRotation != playerPhysics.rotation)
				{
					float pozDistance = 0;
					float negDistance = 0;
					if (playerPhysics.desiredRotation > playerPhysics.rotation)
					{
						pozDistance = playerPhysics.desiredRotation - playerPhysics.rotation;
						negDistance = playerPhysics.rotation + pi2 - playerPhysics.desiredRotation;
					}
					else
					{
						pozDistance = pi2 - playerPhysics.rotation + playerPhysics.desiredRotation;
						negDistance = playerPhysics.rotation - playerPhysics.desiredRotation;
					}

					float speed = input.deltaTime * 3.141592f * 2.f;
					float oldRot = playerPhysics.rotation;
					if (pozDistance > negDistance)
					{
						if (negDistance < speed)
						{
							playerPhysics.rotation = playerPhysics.desiredRotation;
						}
						else
						{
							playerPhysics.rotation -= speed;
						}
					}
					else
					{
						if (pozDistance < speed)
						{
							playerPhysics.rotation = playerPhysics.desiredRotation;
						}
						else
						{
							playerPhysics.rotation += speed;
						}
					}

					if (playerPhysics.rotation > pi2) { playerPhysics.rotation -= pi2; }
					

				}


			}
		#pragma endregion

			resolveConstrains(playerPhysics);
			playerPhysics.updateMove();

			auto setTransform = [&](PhysicsComponent &p, gl3d::Entity &e)
			{
				gl3d::Transform t;
				t.rotation.y = p.rotation;
				t.position = glm::vec3(p.position.x, 12.55, p.position.y);
				t.scale = glm::vec3(0.45);
				renderer.setEntityTransform(e, t);
				return t;
			};

		#pragma region player position
			auto t = setTransform(playerPhysics, player);
			glm::vec3 playerPos = t.position;
			glm::vec3 cameraViewDir = glm::normalize(glm::vec3(-1, 1.7, 1));
			glm::vec3 cameraPos = playerPos + cameraViewDir * 12.f;
		#pragma endregion

			for (auto &e : enemies)
			{
				setTransform(e.physics, e.entity);
			}


		#pragma region sword
			{
				gl3d::Transform t;
				renderer.getEntityJointTransform(player, "arm.r", t);
				t.scale = glm::vec3(0.5);

				gl3d::Transform offset;
				offset.rotation.x = glm::radians(90.f);
				//offset.position.y = -1.f;

				t.setFromMatrix(t.getTransformMatrix() *offset.getTransformMatrix());

				renderer.setEntityTransform(sword, t);
			}
		#pragma endregion


		#pragma region camera hover
			{
				static float cameraYoffset = 0;
				cameraPos.y += cameraYoffset;

				auto checkHit = [&](glm::vec3 pos) -> bool
				{
					for (int x = -2; x <= 2; x++)
						for (int y = -2; y <= 2; y++)
							for (int z = -2; z <= 2; z++)
							{
								glm::vec3 p = pos + glm::vec3(x, y, z);
								if (getBlockSafe(p.x, p.y, p.z) != 0)
								{
									return true;
								}
							}
					return false;
				};
				
				
				float hoverSpeed = 2;
				if (checkHit(cameraPos))
				{
					cameraYoffset += input.deltaTime * hoverSpeed;
				}
				else
				{
					if(cameraYoffset>0)
					if (!checkHit(cameraPos -= glm::vec3(0, 0.1f, 0)))
					{
						cameraYoffset -= input.deltaTime * hoverSpeed;
					}
				}

				cameraYoffset = std::max(cameraYoffset, 0.f);

			}
		#pragma endregion


			if (true)
			{
				renderer.camera.position = cameraPos;
				renderer.camera.viewDirection = glm::normalize(playerPos - cameraPos);
			}
			else
			{
				editor.update(requestedInfo.requestedImguiIds, renderer, input, 4, requestedInfo, {windowState.w,windowState.h});
			}
			

		}
	#pragma endregion



	#pragma endregion



		renderer.render(input.deltaTime);
		glDisable(GL_DEPTH_TEST);




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