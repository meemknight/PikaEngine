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
#include <containers/minecraftDungeons/mcDungeonsgameplay.h>
#include <sushi/sushi.h>
#include <engineLibraresSupport/sushi/engineSushiSupport.h>

//todo engine: don't remove notifications untill the user can see them (the engine has focus)
struct ThreeDGameExample: public Container
{
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


	bool player2joined = 0;

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(1500); //todo option to use global allocator

		info.requestImguiFbo = true;
		info.requestImguiIds = 1;

		return info;
	}

	struct Enemy
	{
		McDungeonsGameplay::PhysicsComponent physics;
		gl3d::Entity entity;
		float life = 1;
		float attackCulldown = 0.f;

		Enemy() {};
		Enemy(int x, int z) { physics.position = {x,z}; physics.lastPos = {x,z}; };
	};

	struct PlayerModel
	{
		gl3d::Entity head;
		gl3d::Entity torso;
		gl3d::Entity hands[2];
		gl3d::Entity legs[2];

		float armAngle = 0;

		gl3d::PointLight pointLight;
		gl3d::SpotLight spotLight;

		McDungeonsGameplay::PhysicsComponent physics;
	};

	PlayerModel playerModel1;
	PlayerModel playerModel2;

	std::vector<Enemy> enemies;
	float enemySpawnTimer = 5;

	gl3d::Renderer3D renderer;
	gl3d::Model groundModel;
	gl3d::Entity groundEntity;
	bool first = 1;

	int killed = 0;

	sushi::SushyContext sushyContext;

	pika::gl3d::General3DEditor editor;

	gl2d::Renderer2D renderer2d;
	gl2d::Font font;

	pika::GL::PikaFramebuffer frameBuffers[2];

	//todo add this in the renderer
	//todo add a default material if material left blank or something
	gl3d::Model createCubeModel(gl3d::Renderer3D &renderer, glm::vec3 color, float metallic = 1)
	{
		static float uv = 1;
		static float cubePositionsNormals[] = {
			-1.0f, +1.0f, +1.0f, // 0
			+0.0f, +1.0f, +0.0f, // Normal
			0, 0,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			+1.0f, +1.0f, +1.0f, // 1
			+0.0f, +1.0f, +0.0f, // Normal
			1 * uv, 0,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			+1.0f, +1.0f, -1.0f, // 2
			+0.0f, +1.0f, +0.0f, // Normal
			1 * uv, 1 * uv,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			-1.0f, +1.0f, -1.0f, // 3
			+0.0f, +1.0f, +0.0f, // Normal
			0, 1 * uv,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent



			-1.0f, +1.0f, -1.0f, // 4
			 0.0f, +0.0f, -1.0f, // Normal
			 0, 1 * uv,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			+1.0f, +1.0f, -1.0f, // 5
			 0.0f, +0.0f, -1.0f, // Normal
			 1 * uv, 1 * uv,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			 +1.0f, -1.0f, -1.0f, // 6
			 0.0f, +0.0f, -1.0f, // Normal
			 1 * uv, 0,				 //uv
			 //0,0,0,				 //tangent
			 //0,0,0,				 //btangent

			-1.0f, -1.0f, -1.0f, // 7
			 0.0f, +0.0f, -1.0f, // Normal
			 0, 0,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			+1.0f, +1.0f, -1.0f, // 8
			+1.0f, +0.0f, +0.0f, // Normal
			1 * uv, 0,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			+1.0f, +1.0f, +1.0f, // 9
			+1.0f, +0.0f, +0.0f, // Normal
			1 * uv, 1 * uv,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			+1.0f, -1.0f, +1.0f, // 10
			+1.0f, +0.0f, +0.0f, // Normal
			0, 1 * uv,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			+1.0f, -1.0f, -1.0f, // 11
			+1.0f, +0.0f, +0.0f, // Normal
			0, 0,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			-1.0f, +1.0f, +1.0f, // 12
			-1.0f, +0.0f, +0.0f, // Normal
			1 * uv, 1 * uv,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			-1.0f, +1.0f, -1.0f, // 13
			-1.0f, +0.0f, +0.0f, // Normal
			1 * uv, 0,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			-1.0f, -1.0f, -1.0f, // 14
			-1.0f, +0.0f, +0.0f, // Normal
			0, 0,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			-1.0f, -1.0f, +1.0f, // 15
			-1.0f, +0.0f, +0.0f, // Normal
			0, 1 * uv,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent


			+1.0f, +1.0f, +1.0f, // 16
			+0.0f, +0.0f, +1.0f, // Normal
			1 * uv, 1 * uv,				 //uv
			//0,0,0,				 //tangent
			//0,0,0,				 //btangent

			-1.0f, +1.0f, +1.0f, // 17
			+0.0f, +0.0f, +1.0f, // Normal
			0, 1 * uv,				 //uv
			//0, 0, 0,				 //tangent
			//0, 0, 0,				 //btangent

			-1.0f, -1.0f, +1.0f, // 18
			+0.0f, +0.0f, +1.0f, // Normal
			0, 0,				 //uv
			//0, 0, 0,				 //tangent
			//0, 0, 0,				 //btangent

			+1.0f, -1.0f, +1.0f, // 19
			+0.0f, +0.0f, +1.0f, // Normal
			1 * uv, 0,				 //uv
			//0, 0, 0,				 //tangent
			//0, 0, 0,				 //btangent


			+1.0f, -1.0f, -1.0f, // 20
			+0.0f, -1.0f, +0.0f, // Normal
			1 * uv, 0,				 //uv
			//0, 0, 0,				 //tangent
			//0, 0, 0,				 //btangent

			-1.0f, -1.0f, -1.0f, // 21
			+0.0f, -1.0f, +0.0f, // Normal
			0, 0,				 //uv
			//0, 0, 0,				 //tangent
			//0, 0, 0,				 //btangent

			-1.0f, -1.0f, +1.0f, // 22
			+0.0f, -1.0f, +0.0f, // Normal
			0, 1 * uv,				 //uv
			//0, 0, 0,				 //tangent
			//0, 0, 0,				 //btangent

			+1.0f, -1.0f, +1.0f, // 23
			+0.0f, -1.0f, +0.0f, // Normal
			1 * uv, 1 * uv,				 //uv
			//0, 0, 0,				 //tangent
			//0, 0, 0,				 //btangent

		};

		static unsigned int cubeIndices[] = {
		0,   1,  2,  0,  2,  3, // Top
		4,   5,  6,  4,  6,  7, // Back
		8,   9, 10,  8, 10, 11, // Right
		12, 13, 14, 12, 14, 15, // Left
		16, 17, 18, 16, 18, 19, // Front
		20, 22, 21, 20, 23, 22, // Bottom
		};

		auto material = renderer.createMaterial(0, {color,1}, 0, metallic);

		return renderer.createModelFromData(material, "cube",
			sizeof(cubePositionsNormals)/sizeof(cubePositionsNormals[0]), cubePositionsNormals,
			sizeof(cubeIndices)/ sizeof(cubeIndices[0]), cubeIndices);

	}

	void createPlayerEntity(gl3d::Renderer3D &renderer, glm::vec3 color, PlayerModel& playerModel)
	{
		auto cube = createCubeModel(renderer, color, 0);

		playerModel.torso = renderer.createEntity(cube, {}, false, true, false);

		playerModel.legs[0] = renderer.createEntity(cube, {}, false, true, false);
		playerModel.legs[1] = renderer.createEntity(cube, {}, false, true, false);

		playerModel.hands[0] = renderer.createEntity(cube, {}, false, true, false);
		playerModel.hands[1] = renderer.createEntity(cube, {}, false, true, false);

		playerModel.head = renderer.createEntity(cube, {}, false, true, false);

		playerModel.pointLight = renderer.createPointLight({}, glm::vec3{1.f}, 20, 1);
		playerModel.spotLight = renderer.createSpotLight({}, glm::radians(50.f), {0,0}, 40, 10, glm::vec3(2.f));
	}

	gl3d::Model createPlane(gl3d::Renderer3D &renderer)
	{
		float uv = 10;
		float size = 200;
		std::vector<unsigned int> ind = {0,   1,  2,  0,  2,  3};
		std::vector<float> topVer = {
				-1.0f * size, +0.0f, +1.0f * size, // 0
				+0.0f, +1.0f, +0.0f, // Normal
				0, 0,				 //uv

				+1.0f * size, +0.0f, +1.0f * size, // 1
				+0.0f, +1.0f, +0.0f, // Normal
				1 * uv, 0,				 //uv

				+1.0f * size, +0.0f, -1.0f * size, // 2
				+0.0f, +1.0f, +0.0f, // Normal
				1 * uv, 1 * uv,				 //uv

				-1.0f * size, +0.0f, -1.0f * size, // 3
				+0.0f, +1.0f, +0.0f, // Normal
				0, 1 * uv,			 //uv
		};

		//auto material =  renderer.createMaterial(0, {1,1,1,1}, 0, 1);
		auto material =  renderer.loadMaterial(PIKA_RESOURCES_PATH "materials/rustedIron.mtl", gl3d::maxQuality);

		if (material.empty())
		{
			material.push_back(renderer.createMaterial(0, {1,1,1,1}, 0, 1));
		}

		return renderer.createModelFromData(material[0], "plane",
			topVer.size(), topVer.data(), ind.size(),
			ind.data());
	}


	void createStalpi()
	{
		auto cube = createCubeModel(renderer, {0.2,0.4,0.8}, 0);

		bool flip = 0;
		for (int x = -100; x <= 100; x += 25)
		{
			for (int z = -100; z <= 100; z+=50)
			{

				addStalp({x, z + 25 * flip}, cube);

			}

			flip = !flip;
		}

	}

	std::vector<gl3d::SpotLight> spotLights;

	void addStalp(glm::vec2 position, gl3d::Model cube)
	{
		renderer.createEntity(cube, {glm::vec3{position.x, 3.5, position.y}
			,{},{0.25,5,0.25}
			});

		renderer.createEntity(cube, {glm::vec3{position.x, 8.5, position.y}
			,{},{3,0.25,0.25}
			});

		spotLights.push_back(renderer.createSpotLight({position.x - 1.5, 8.1, position.y},
			glm::radians(55.f), glm::vec3(0, -1, 0), 20, 2, glm::vec3(1.f), 1.f, 0));

		spotLights.push_back
		(renderer.createSpotLight({position.x + 1.5, 8.1, position.y},
			glm::radians(55.f), glm::vec3(0, -1, 0), 20, 2, glm::vec3(1.f), 1.f, 0));


	}

	void addEnemy(glm::vec2 position, gl3d::Model playerM, gl3d::Material mat)
	{
		Enemy i = {};
		i.physics.position = position; i.physics.lastPos = position;

		i.entity = renderer.createEntity(playerM,
			gl3d::Transform{glm::vec3{i.physics.position.x, 0, i.physics.position.y}
			,{},glm::vec3{0.5f}
			}, false);


		int count = renderer.getEntityMeshesCount(i.entity);
		//if (!count)return 0;

		for (int j = 0; j < count; j++)
		{
			renderer.setEntityMeshMaterial(i.entity, j, mat);

			auto m = renderer.getEntityMeshMaterialValues(i.entity, j);
			m.kd = {(rand()%255)/255.f,(rand() % 255) / 255.f,(rand() % 255) / 255.f,1};
			renderer.setEntityMeshMaterialValues(i.entity, j, m);
		}

		renderer.setEntityAnimationIndex(i.entity, 15); //idle
		renderer.setEntityAnimationSpeed(i.entity, 1);
		renderer.setEntityAnimate(i.entity, true);

		enemies.push_back(i);
	}

	gl3d::Model playerM;
	std::vector<gl3d::Material> zombieMat;

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		
		renderer2d.create(requestedInfo.requestedFBO.fbo, 50);
		//font = pika::gl2d::loadFont(PIKA_RESOURCES_PATH "arial.ttf", requestedInfo);

		frameBuffers[0].createFramebuffer(1, 1, true);
		frameBuffers[1].createFramebuffer(1, 1, true);
		
		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;

		renderer.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", requestedInfo.requestedFBO.fbo);

		renderer.skyBox = renderer.loadSkyBox(PIKA_RESOURCES_PATH "/skyBoxes/forest.png");

		//pika::gl3d::loadSettingsFromFileName(renderer, PIKA_RESOURCES_PATH "/threedgame/settings.gl3d", requestedInfo);
		editor.loadFromFile(renderer, PIKA_RESOURCES_PATH "/threedgame/settings.gl3d", requestedInfo);
		//renderer.skyBox.color = glm::vec3(0.8f);


		zombieMat = renderer.loadMaterial(PIKA_RESOURCES_PATH "threedgame/zombie.mtl", 0);
		if (zombieMat.size() != 1) { return false; }


		playerM = renderer.loadModel(PIKA_RESOURCES_PATH "mcDungeons/steve.glb", 0, 1);

		renderer.getSSRdata().setLowQuality();

		renderer.getDirectionalShadowCascadesFrustumSplit(0) = 0.15;
		renderer.getDirectionalShadowCascadesFrustumSplit(1) = 0.28;
		renderer.getDirectionalShadowCascadesFrustumSplit(2) = 0.48;


		//renderer.

		createPlayerEntity(renderer, {1,1,0}, playerModel1);

		createStalpi();

		groundModel = createPlane(renderer);

	

		gl3d::Transform t;
		t.position = {0, -0.4, 0};
		//t.rotation = {1.5, 0 , 0};

		groundEntity = renderer.createEntity(groundModel, t);


		renderer.createDirectionalLight(glm::normalize(glm::vec3{-0.175,-0.577, -0.798}), glm::vec3{0.25f});
	

		{
			sushi::SushyBinaryFormat loader;
			requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "threedgame/ui.sushi", loader.data);
			sushyContext.load(loader);
			sushyContext.root.background.color = {0,0,0,0};
		}

		//if (0)
		{
			addEnemy({18,16}, playerM, zombieMat[0]);
			addEnemy({16,16}, playerM, zombieMat[0]);
		}

		return true;
	}

	float attackCulldown = 0;
	bool freeCamera = 0;

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
	#pragma region init stuff
		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_MULTISAMPLE);

		//renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
		//renderer.camera.aspectRatio = (float)windowState.windowW / windowState.windowH; //todo do this in update

		renderer2d.updateWindowMetrics(windowState.windowW, windowState.windowH);
	#pragma endregion

		frameBuffers[0].clear();
		frameBuffers[0].resizeFramebuffer(windowState.windowW / 2, windowState.windowH);
		frameBuffers[1].clear();
		frameBuffers[1].resizeFramebuffer(windowState.windowW/2, windowState.windowH);


		if (!player2joined && input.buttons[pika::Button::Enter].pressed())
		{
			player2joined = true;
			createPlayerEntity(renderer, {1,0,0}, playerModel2);

		}

		if (enemySpawnTimer < 0)
		{
			
			if (enemies.size() < 15)
			{
				addEnemy({rand() % 100 - 50, rand() % 100 - 50}, playerM, zombieMat[0]);
			}


		}
		else
		{
			enemySpawnTimer -= input.deltaTime;
		}

		if (input.buttons[pika::Button::Escape].released())	
		{
			::pika::pikaImgui::removeFocusToCurrentWindow();
			requestedInfo.setNormalCursor();
		}

		if (input.buttons[pika::Button::F].released())
		{
			freeCamera = !freeCamera;
		}

		//editor.update(requestedInfo.requestedImguiIds, renderer, input, 4, requestedInfo, {windowState.windowW,windowState.windowH});
	
	#pragma region spot lights

		for (int i = 0; i < spotLights.size(); i++)
		{

			//auto t = renderer.getSpotLightDirection(spotLights[i]);

			renderer.setSpotLightDirection(spotLights[i], gl3d::fromAnglesToDirection(glm::radians(190.f), 
				((i%2)*2-1) * clock()*0.0003f + i));

		}

	#pragma endregion


	#pragma region player
		{

		#pragma region input

			
			{
				if (!freeCamera)
				{

					auto updatePlayer = [&](PlayerModel &p, bool differentInput = 0)
					{

						glm::vec2 dir = {};

						{
							if (input.buttons[pika::Button::A].held() || input.buttons[pika::Button::Left].held())
							{
								dir -= glm::vec2(1, 1);
							}
							if (input.buttons[pika::Button::D].held() || input.buttons[pika::Button::Right].held())
							{
								dir += glm::vec2(1, 1);
							}
							if (input.buttons[pika::Button::W].held() || input.buttons[pika::Button::Up].held())
							{
								dir += glm::vec2(1, -1);
							}
							if (input.buttons[pika::Button::S].held() || input.buttons[pika::Button::Down].held())
							{
								dir -= glm::vec2(1, -1);
							}
						}

						if (dir.x != 0 || dir.y != 0)
						{
							dir = glm::normalize(dir);

							p.physics.desiredRotation = std::atan2(dir.x, dir.y);
							
							p.armAngle = sin(clock()/100.f);

							{
								//renderer.setEntityAnimationIndex(playerEntity, run); //run 
								//renderer.setEntityAnimationSpeed(playerEntity, 1.5);
							}
						}
						else
						{
							if (p.armAngle > 0)
							{
								p.armAngle -= input.deltaTime;
								if (p.armAngle < 0) p.armAngle = 0;

							}
							else if(p.armAngle < 0)
							{
								p.armAngle += input.deltaTime;
								if (p.armAngle > 0) p.armAngle = 0;
							}

							{
								//renderer.setEntityAnimationIndex(playerEntity, idle); //idle
								//renderer.setEntityAnimationSpeed(playerEntity, 1);
							}
						}

						float speed = 3;
						p.physics.position += dir * input.deltaTime * speed;


						if (input.buttons[pika::Button::Space].pressed() && attackCulldown <= 0)
						{
							//renderer.setEntityAnimationIndex(playerEntity, attack); //attack
							//renderer.setEntityAnimationSpeed(playerEntity, 1.3);
							attackCulldown = 0.2;

							for (int i = 0; i < enemies.size(); i++)
							{
								float d = glm::distance(p.physics.position, enemies[i].physics.position);
								if (d < 1.5f)
								{
									enemies[i].life -= 0.4;

									if (enemies[i].life <= 0)
									{
										renderer.deleteEntity(enemies[i].entity);
										enemies.erase(enemies.begin() + i);
										killed++;
										i--;
										continue;
									}

									glm::vec2 dir(1, 0);

									if (d != 0)
									{
										dir = (enemies[i].physics.position - p.physics.position) / d;
									}

									enemies[i].physics.position += dir * 4.0f;
								}

							}

						}

						if (attackCulldown > 0)
						{
							attackCulldown -= input.deltaTime;
						}

						//set player's position
						{
							auto setPos = [&](PlayerModel &p)
							{
								float limbRotation = p.armAngle;

								renderer.setEntityTransform(p.torso,
									{{p.physics.position.x, 2.50, p.physics.position.y}, {0,
									p.physics.rotation, 0}, {0.5,1,0.25}});

								renderer.setEntityTransform(p.head,
									{{p.physics.position.x, 3.0, p.physics.position.y}, {0,
									p.physics.rotation, 0}, {0.5,0.5,0.5}});

								glm::mat4 legMatrix =
									glm::translate(glm::vec3{p.physics.position.x, 0.75, p.physics.position.y}) *
									glm::rotate(p.physics.rotation, glm::vec3{0,0.75,0}) *
									glm::translate(glm::vec3{0, 0.75 / 2.f, 0}) *
									glm::rotate(limbRotation, glm::vec3{1,0,0}) *
									glm::translate(glm::vec3{-0.25, -0.75 / 2.f, 0}) *
									glm::scale(glm::vec3{0.25,0.75,0.25});

								gl3d::Transform legTransform;
								legTransform.setFromMatrix(legMatrix);

								glm::mat4 legMatrix2 =
									glm::translate(glm::vec3{p.physics.position.x, 0.75, p.physics.position.y}) *
									glm::rotate(p.physics.rotation, glm::vec3{0,0.75,0}) *
									glm::translate(glm::vec3{0, 0.75 / 2.f, 0}) *
									glm::rotate(-limbRotation, glm::vec3{1,0,0}) *
									glm::translate(glm::vec3{0.25, -0.75 / 2.f, 0}) *
									glm::scale(glm::vec3{0.25,0.75,0.25});

								gl3d::Transform legTransform2;
								legTransform2.setFromMatrix(legMatrix2);
								renderer.setEntityTransform(p.legs[0], legTransform);
								renderer.setEntityTransform(p.legs[1], legTransform2);


								glm::mat4 armMatrix =
									glm::translate(glm::vec3{p.physics.position.x, 1.75, p.physics.position.y}) *
									glm::rotate(p.physics.rotation, glm::vec3{0,0.75,0}) *
									glm::translate(glm::vec3{0, 0.75 / 2.f, 0}) *
									glm::rotate(-limbRotation, glm::vec3{1,0,0}) *
									glm::translate(glm::vec3{-0.75, -0.75 / 2.f, 0}) *
									glm::scale(glm::vec3{0.25,0.75,0.25});
								gl3d::Transform armTransform;
								armTransform.setFromMatrix(armMatrix);

								glm::mat4 armMatrix2 =
									glm::translate(glm::vec3{p.physics.position.x, 1.75, p.physics.position.y}) *
									glm::rotate(p.physics.rotation, glm::vec3{0,0.75,0}) *
									glm::translate(glm::vec3{0, 0.75 / 2.f, 0}) *
									glm::rotate(limbRotation, glm::vec3{1,0,0}) *
									glm::translate(glm::vec3{0.75, -0.75 / 2.f, 0}) *
									glm::scale(glm::vec3{0.25,0.75,0.25});
								gl3d::Transform armTransform2;
								armTransform2.setFromMatrix(armMatrix2);


								renderer.setEntityTransform(p.hands[0], armTransform);
								renderer.setEntityTransform(p.hands[1], armTransform2);




							};

							setPos(p);
						}

						renderer.setPointLightPosition(p.pointLight, glm::vec3(p.physics.position.x,
							1, p.physics.position.y));

						renderer.setSpotLightPosition(p.spotLight, glm::vec3(p.physics.position.x,
							1, p.physics.position.y));

						renderer.setSpotLightDirection(p.spotLight,
							gl3d::fromAnglesToDirection(glm::radians(90.f), -p.physics.rotation
							+ glm::radians(180.f)));
					};

					updatePlayer(playerModel1, false);

					if (player2joined)
					{
						updatePlayer(playerModel2, true);
					}
					

				}
			}

			
			

		#pragma endregion

			auto solveRotation = [deltaTime = input.deltaTime](McDungeonsGameplay::PhysicsComponent &p)
			{
				const float pi2 = 3.1415926f * 2.f;

				if (p.desiredRotation != p.rotation)
				{
					float pozDistance = 0;
					float negDistance = 0;
					if (p.desiredRotation > p.rotation)
					{
						pozDistance = p.desiredRotation - p.rotation;
						negDistance = p.rotation + pi2 - p.desiredRotation;
					}
					else
					{
						pozDistance = pi2 - p.rotation + p.desiredRotation;
						negDistance = p.rotation - p.desiredRotation;
					}

					float speed = deltaTime * 3.141592f * 2.f;
					float oldRot = p.rotation;
					if (pozDistance > negDistance)
					{
						if (negDistance < speed)
						{
							p.rotation = p.desiredRotation;
						}
						else
						{
							p.rotation -= speed;
						}
					}
					else
					{
						if (pozDistance < speed)
						{
							p.rotation = p.desiredRotation;
						}
						else
						{
							p.rotation += speed;
						}
					}

					if (p.rotation > pi2) { p.rotation -= pi2; }


				}

			};

			solveRotation(playerModel1.physics);
			playerModel1.physics.updateMove();

			if (player2joined)
			{
				solveRotation(playerModel2.physics);
				playerModel2.physics.updateMove();
			}

			auto setTransform = [&](McDungeonsGameplay::PhysicsComponent &p, gl3d::Entity &e)
			{
				gl3d::Transform t;
				t.rotation.y = p.rotation;
				t.position = glm::vec3(p.position.x, -0.5, p.position.y);
				t.scale = glm::vec3(1.f);
				renderer.setEntityTransform(e, t);
				return t;
			};

		#pragma region player position
			glm::vec3 playerPos = {playerModel1.physics.position.x, -0.5, playerModel1.physics.position.y};
			glm::vec3 cameraPos = {};
			{
				glm::vec3 cameraViewDir = glm::normalize(glm::vec3(-1, 0.8, 1));
				cameraPos = playerPos + cameraViewDir * 20.f;
			}
		#pragma endregion

			for (auto &e : enemies)
			{
				float d = glm::distance(e.physics.position, playerModel1.physics.position);
				if (d < 8.f)
				{
					if (d > 1.f)
					{
						glm::vec2 dir = glm::normalize(playerModel1.physics.position - e.physics.position);
						float speed = 1.4;
						e.physics.position += dir * input.deltaTime * speed;
			
						e.physics.desiredRotation = std::atan2(dir.x, dir.y);
			
						renderer.setEntityAnimationIndex(e.entity, zombieRun); //run 
						renderer.setEntityAnimationSpeed(e.entity, 1.5);
			
					}
					else
					{
						renderer.setEntityAnimationIndex(e.entity, zombieAttack); //attack
						renderer.setEntityAnimationSpeed(e.entity, 1.3);
			
						if (e.attackCulldown <= 0.f)
						{
							//health -= 0.1;
							e.attackCulldown = 1.f;
						}
						//attack
					}
				}
				else
				{
					renderer.setEntityAnimationIndex(e.entity, zombieIdle); //idle
					renderer.setEntityAnimationSpeed(e.entity, 1);
				}
			
				if (e.attackCulldown > 0)
				{
					e.attackCulldown -= input.deltaTime;
				}
			
				solveRotation(e.physics);
			
				//resolveConstrains(e.physics);
				e.physics.updateMove();
			
				setTransform(e.physics, e.entity);

				//requestedInfo.consoleWrite(std::to_string(e.physics.position.x) + " " +
				//	std::to_string(e.physics.position.y) + "\n");
			}


			if (!freeCamera)
			{
				renderer.camera.position = cameraPos;
				renderer.camera.viewDirection = glm::normalize(playerPos - cameraPos);
			}
			else
			{
				editor.update(requestedInfo.requestedImguiIds, renderer, input,
					4, requestedInfo, {windowState.windowW,windowState.windowH});
			}


		}
	#pragma endregion



	#pragma region render3d

		if (player2joined && !freeCamera)
		{

			renderer.frameBuffer = frameBuffers[0].fbo;
			renderer.updateWindowMetrics(windowState.windowW/2, windowState.windowH);
			renderer.camera.aspectRatio = (float)(windowState.windowW/2) / windowState.windowH;
			renderer.render(input.deltaTime);

			renderer.frameBuffer = frameBuffers[1].fbo;
			//renderer.updateWindowMetrics(windowState.windowW / 2, windowState.windowH);
			renderer.camera.aspectRatio = (float)(windowState.windowW / 2) / windowState.windowH;
			renderer.render(0);

			gl2d::Texture t1; t1.id = frameBuffers[0].texture;
			gl2d::Texture t2; t2.id = frameBuffers[1].texture;
			renderer2d.renderRectangle({0,0, windowState.windowW / 2, windowState.windowH}, t1);
			renderer2d.renderRectangle({windowState.windowW / 2,0, windowState.windowW / 2, windowState.windowH}, t2);

			glDisable(GL_DEPTH_TEST);
			renderer2d.flush();

			//todo constructor (maybe explicit from uint)
		}
		else
		{
			renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
			renderer.frameBuffer = requestedInfo.requestedFBO.fbo;
			renderer.render(input.deltaTime);
		}




		glDisable(GL_DEPTH_TEST);
	#pragma endregion




		return true;
	}

	void destruct(RequestedContainerInfo &requestedInfo) override
	{
		renderer.clearAllRendererResources();

		frameBuffers[0].deleteFramebuffer();
		frameBuffers[1].deleteFramebuffer();

		renderer2d.cleanup();
		font.texture.cleanup();

	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release

//todo reset container button (keep same imgui id)

