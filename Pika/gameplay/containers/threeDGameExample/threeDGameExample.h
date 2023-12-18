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

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(1500); //todo option to use global allocator

		info.extensionsSuported = {".gl3d"};

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


	std::vector<Enemy> enemies;

	gl3d::Renderer3D renderer;
	gl3d::Model groundModel;
	gl3d::Entity groundEntity;
	gl3d::Entity playerEntity;
	bool first = 1;
	float health = 1;

	int killed = 0;

	float floatingPeriod = 0;
	float killTimer = 1.5;

	sushi::SushyContext sushyContext;

	pika::gl3d::General3DEditor editor;

	gl2d::Renderer2D renderer2d;
	gl2d::Font font;


	gl3d::Model createPlane(gl3d::Renderer3D &renderer)
	{
		float uv = 1;
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

		auto material =  renderer.createMaterial(0, {0.4,1,0.4,1}, 0, 1);

		return renderer.createModelFromData(material, "plane",
			topVer.size(), topVer.data(), ind.size(),
			ind.data());
	}


	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		renderer2d.create(requestedInfo.requestedFBO.fbo);
		font = pika::gl2d::loadFont(PIKA_RESOURCES_PATH "arial.ttf", requestedInfo);

		
		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;

		renderer.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", requestedInfo.requestedFBO.fbo);

		renderer.skyBox = renderer.loadSkyBox(PIKA_RESOURCES_PATH "/skyBoxes/forest.png");

		//pika::gl3d::loadSettingsFromFileName(renderer, PIKA_RESOURCES_PATH "/threedgame/settings.gl3d", requestedInfo);
		editor.loadFromFile(renderer, PIKA_RESOURCES_PATH "/threedgame/settings.gl3d", requestedInfo);

		auto zombieMat = renderer.loadMaterial(PIKA_RESOURCES_PATH "threedgame/zombie.mtl", 0);
		if (zombieMat.size() != 1) { return false; }

		auto playerModel = renderer.loadModel(PIKA_RESOURCES_PATH "mcDungeons/steve.glb", 0, 1);

		renderer.getSSRdata().setLowQuality();

		renderer.getDirectionalShadowCascadesFrustumSplit(0) = 0.15;
		renderer.getDirectionalShadowCascadesFrustumSplit(1) = 0.28;
		renderer.getDirectionalShadowCascadesFrustumSplit(2) = 0.48;


		//renderer.

		groundModel = createPlane(renderer);

	

		gl3d::Transform t;
		t.position = {0, -0.4, 0};
		//t.rotation = {1.5, 0 , 0};

		groundEntity = renderer.createEntity(groundModel, t);


		playerEntity = renderer.createEntity(playerModel, gl3d::Transform{{0,0,-5},{},glm::vec3{0.5f}}, false);

		renderer.createDirectionalLight(glm::normalize(glm::vec3{-0.175,-0.577, -0.798}));
	
		renderer.setEntityAnimate(playerEntity, 1);
		renderer.setEntityAnimationIndex(playerEntity, idle);


		{
			sushi::SushyBinaryFormat loader;
			requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "threedgame/ui.sushi", loader.data);
			sushyContext.load(loader);
			sushyContext.root.background.color = {0,0,0,0};
		}

		//if (0)
		{
			enemies.push_back(Enemy(18, 16));
			enemies.push_back(Enemy(16, 16));
		}

		for (auto &i : enemies)
		{
			i.entity = renderer.createEntity(playerModel, 
				gl3d::Transform{glm::vec3{i.physics.position.x, 0, i.physics.position.y}
				,{},glm::vec3{0.5f}
				}, false);


			int count = renderer.getEntityMeshesCount(i.entity);
			if (!count)return 0;
			for (int j = 0; j < count; j++)
			{
				renderer.setEntityMeshMaterial(i.entity, j, zombieMat[0]);

				auto m = renderer.getEntityMeshMaterialValues(i.entity, j);
				m.kd = {1,0,0,1};
				renderer.setEntityMeshMaterialValues(i.entity, j, m);
			}

			renderer.setEntityAnimationIndex(i.entity, 15); //idle
			renderer.setEntityAnimationSpeed(i.entity, 1);
			renderer.setEntityAnimate(i.entity, true);
		}

		return true;
	}

	float animationCulldown = 0;
	float attackCulldown = 0;
	float cameraYoffset = 0;
	bool freeCamera = 0;

	McDungeonsGameplay::PhysicsComponent playerPhysics;

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

		renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
		renderer.camera.aspectRatio = (float)windowState.windowW / windowState.windowH; //todo do this in update

		renderer2d.updateWindowMetrics(windowState.windowW, windowState.windowH);
	#pragma endregion

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

	#pragma region player
		{

		#pragma region input

			if (health <= 0)
			{
				renderer.setEntityAnimationIndex(playerEntity, 18); //die
				renderer.setEntityAnimationSpeed(playerEntity, 0.5);

				killTimer -= input.deltaTime;

				if (killTimer <= 0)
				{
					requestedInfo.setNormalCursor(); //todo set by the engine on exit container
					requestedInfo.createContainer("ThreeDGameMenu");
					return 0;
				}
			}
			else
			{
				if (!freeCamera)
				{
					glm::vec2 dir = {};

					if (animationCulldown <= 0)
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

						auto lerp = [&](float x, float y, float a)
						{
							a = std::min(a, 1.f);
							a = std::max(a, 0.f);
							return x * (1 - a) + y * (a);
						};

						playerPhysics.desiredRotation = std::atan2(dir.x, dir.y);

						if (animationCulldown <= 0)
						{
							renderer.setEntityAnimationIndex(playerEntity, run); //run 
							renderer.setEntityAnimationSpeed(playerEntity, 1.5);
						}
					}
					else
					{
						if (animationCulldown <= 0)
						{
							renderer.setEntityAnimationIndex(playerEntity, idle); //idle
							renderer.setEntityAnimationSpeed(playerEntity, 1);
						}
					}

					float speed = 3;
					playerPhysics.position += dir * input.deltaTime * speed;


					if (input.buttons[pika::Button::Space].pressed() && attackCulldown <= 0)
					{
						renderer.setEntityAnimationIndex(playerEntity, attack); //attack
						renderer.setEntityAnimationSpeed(playerEntity, 1.3);
						animationCulldown = 1.2;
						attackCulldown = 1.2;

						for (int i = 0; i < enemies.size(); i++)
						{
							float d = glm::distance(playerPhysics.position, enemies[i].physics.position);
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
									dir = (enemies[i].physics.position - playerPhysics.position) / d;
								}

								enemies[i].physics.position += dir * 4.0f;
							}

						}

					}

				}

				if (animationCulldown > 0)
				{
					animationCulldown -= input.deltaTime;
				}

				if (attackCulldown > 0)
				{
					attackCulldown -= input.deltaTime;
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

			solveRotation(playerPhysics);

			//resolveConstrains(playerPhysics);
			playerPhysics.updateMove();

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
			auto t = setTransform(playerPhysics, playerEntity);
			glm::vec3 playerPos = t.position;
			glm::vec3 cameraViewDir = glm::normalize(glm::vec3(-1, 0.8, 1));
			glm::vec3 cameraPos = playerPos + cameraViewDir * 20.f;
		#pragma endregion

			for (auto &e : enemies)
			{
				float d = glm::distance(e.physics.position, playerPhysics.position);
				if (d < 8.f)
				{
					if (d > 1.f)
					{
						glm::vec2 dir = glm::normalize(playerPhysics.position - e.physics.position);
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


#pragma region sword
	//{
	//	gl3d::Transform t;
	//	renderer.getEntityJointTransform(player, "arm.r", t);
	//	t.scale = glm::vec3(0.5);
	//
	//	gl3d::Transform offset;
	//	offset.rotation.x = glm::radians(90.f);
	//	//offset.position.y = -1.f;
	//
	//	t.setFromMatrix(t.getTransformMatrix() * offset.getTransformMatrix());
	//
	//	renderer.setEntityTransform(sword, t);
	//}
#pragma endregion


#pragma region camera hover
			{
			
			if(health<=0)
			{
				cameraYoffset += input.deltaTime;
			}
			//
			//
			cameraYoffset = std::max(cameraYoffset, 0.f);
			cameraPos.y += cameraYoffset;
			//cameraPos.x = std::max(cameraPos.x, 2.f);

			}

		#pragma endregion

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



		health = glm::clamp(health, 0.f, 1.f);


	#pragma region render3d

		renderer.render(input.deltaTime);
		glDisable(GL_DEPTH_TEST);
	#pragma endregion


	#pragma region ui
		if(!freeCamera)
		{
			sushyContext.update(renderer2d, pika::toSushi(input), font);
			
			auto life = sushyContext.genUniqueParent("life");
			if (life)
			{
				auto t2 = life->outData.absTransform;
				t2.z *= health;

				renderer2d.renderRectangle(life->outData.absTransform, {0.5,0.5,0.5,0.1});
				renderer2d.renderRectangle(t2, {1,0,0,0.5});
			}
			
			//auto xp = sushyContext.genUniqueParent("xp");
			//if (xp)
			//{
			//	renderer2d.renderRectangle(xp->outData.absTransform, {0,1,0,0.5});
			//}
			
			auto score = sushyContext.genUniqueParent("score");
			if (score)
			{
				renderer2d.renderRectangle(score->outData.absTransform, {0.5,0.5,0.5,0.3});

				auto s = renderer2d.determineTextRescaleFit(std::to_string(killed).c_str(), font,
					score->outData.absTransform);

				renderer2d.renderText(glm::vec2(score->outData.absTransform) + glm::vec2{score->outData.absTransform.z,score->outData.absTransform.w} / 2.f,
					std::to_string(killed).c_str(), font, Colors_White);

			}
			
			renderer2d.flush();
		}
	#pragma endregion



		return true;
	}

	void destruct(RequestedContainerInfo &requestedInfo) override
	{
		renderer.clearAllRendererResources();
	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release

//todo reset container button (keep same imgui id)

