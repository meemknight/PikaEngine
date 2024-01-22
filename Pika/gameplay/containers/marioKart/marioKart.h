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
#include <containers/marioKart/marioKartEditor.h>

struct MarioKart: public Container
{



	std::vector<MarioKartEditor::RoadMarker> markers;
	int currentMarker = 0;
	glm::vec2 markerDelta = {};

	float acceleration = 0;

	bool freeCamera = 0;
	bool showGuides = 0;
	
	std::vector<float> coinTimers;
	int score = 0;


	gl2d::Font font;
	gl2d::Renderer2D renderer2d;


	struct Pilot
	{
		gl3d::Entity car;
		gl3d::Entity character;
		gl3d::Entity wheels[4];
		glm::vec3 carPosition = {};
		glm::vec3 moveDirection = glm::normalize(glm::vec3{0,0,-1});
		int carMarker = 0;
	};

	struct GameplayData
	{
		int gameplayPhaze = 0; //0 = animation
		float animationTimer = 9.f;

	}gameplayData;

	Pilot player;

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(1000); //todo option to use global allocator

		info.extensionsSuported = {""};

		info.requestImguiFbo = true;
		info.requestImguiIds = 1;

		return info;
	}

	float cameraSpeed = 50.f;
	gl3d::Renderer3D renderer;
	gl3d::Model worldModel;
	gl3d::Model sphereModel;
	gl3d::Model wheelModel;
	gl3d::Model marioModel;
	gl3d::Model bowserModel;
	gl3d::Model booModel;
	gl3d::Entity worldEntity;
	gl3d::Model carModel;
	gl3d::Model coinModel;

	float spinTimer = 0;

	std::vector<gl3d::Entity> spheres;
	std::vector<gl3d::Entity> coins;


	pika::gl3d::General3DEditor editor;

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		renderer2d.create(requestedInfo.requestedFBO.fbo);

		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;
		renderer.frustumCulling = false;
		renderer.tonemapper = 2;

		renderer.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", requestedInfo.requestedFBO.fbo);
		

		font = pika::gl2d::loadFont(PIKA_RESOURCES_PATH "mcDungeons/CommodorePixeled.ttf", requestedInfo);

		//const char *names[6] =
		//{PIKA_RESOURCES_PATH "/skyBoxes/ocean/right.jpg",
		//	PIKA_RESOURCES_PATH "/skyBoxes/ocean/left.jpg",
		//	PIKA_RESOURCES_PATH "/skyBoxes/ocean/top.jpg",
		//	PIKA_RESOURCES_PATH "/skyBoxes/ocean/bottom.jpg",
		//	PIKA_RESOURCES_PATH "/skyBoxes/ocean/front.jpg",
		//	PIKA_RESOURCES_PATH "/skyBoxes/ocean/back.jpg"};
		
		//renderer.skyBox = renderer.loadSkyBox(names);
		//renderer.skyBox.color = {0.2,0.3,0.8};
		//renderer.skyBox = renderer.atmosfericScattering({0,1,0}, {0.2,0.2,0.5}, {0.6,0.2,0.1}, {},
		//	false, 10);
		renderer.skyBox = renderer.loadSkyBox(PIKA_RESOURCES_PATH "/skyBoxes/skybox.png");
		renderer.skyBox.color = glm::vec3(240.f / 255.f);

		editor.loadFromFile(renderer, PIKA_RESOURCES_PATH "/marioKart/settings.gl3d", requestedInfo);
		renderer.internal.lightShader.useTheHbaoImplementation = true;


		renderer.createDirectionalLight({-0.155,-0.907,0.391}, glm::vec3{270.f/255.f});
		renderer.getDirectionalShadowCascadesFrustumSplit(0) = 0.10;
		renderer.getDirectionalShadowCascadesFrustumSplit(1) = 0.150;
		renderer.getDirectionalShadowCascadesFrustumSplit(2) = 0.640;
		renderer.camera.farPlane = 250;

		//helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "helmet/helmet.obj");
		//worldModel = renderer.loadModel("C:/Users/meemk/Desktop/map2/N64 Royal Raceway.obj",
		//	gl3d::TextureLoadQuality::maxQuality, 1);
		worldModel = renderer.loadModel("C:/Users/meemk/Desktop/map2/MarioRaceEnv2.obj",
			gl3d::TextureLoadQuality::maxQuality, 1);
		
		carModel = renderer.loadModel("C:/Users/meemk/Desktop/car/Standard Kart.obj",
			gl3d::TextureLoadQuality::maxQuality, 1.f);

		marioModel = renderer.loadModel("C:/Users/meemk/Desktop/mario/mario2.obj",
			gl3d::TextureLoadQuality::maxQuality, 1.f);

		bowserModel = renderer.loadModel("C:/Users/meemk/Desktop/Bowser/bowser.obj",
			gl3d::TextureLoadQuality::maxQuality, 1.f);

		booModel = renderer.loadModel("C:/Users/meemk/Desktop/Boo/boo.obj",
			gl3d::TextureLoadQuality::maxQuality, 1.f);
		
		coinModel = renderer.loadModel("C:/Users/meemk/Desktop/coin/coin.obj",
			gl3d::TextureLoadQuality::maxQuality, 1.f);

		//todo error by gl3d when creating an entity with no model loaded
		sphereModel = renderer.loadModel(PIKA_RESOURCES_PATH "/marioKart/sphere.obj",
			gl3d::TextureLoadQuality::maxQuality, 1);

		wheelModel = renderer.loadModel("C:/Users/meemk/Desktop/wheel/wheel2.glb", 
			gl3d::TextureLoadQuality::maxQuality, 1);


		worldEntity = renderer.createEntity(worldModel, {});


		player.car = renderer.createEntity(carModel, {}, false);
		player.character = renderer.createEntity(marioModel, {}, false);

		size_t size = 0;
		if (requestedInfo.getFileSizeBinary(PIKA_RESOURCES_PATH "/marioKart/markers.bin", size))
		{

			if (size % sizeof(MarioKartEditor::RoadMarker) == 0 && size)
			{
				markers.resize(size / sizeof(MarioKartEditor::RoadMarker));

				requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "/marioKart/markers.bin",
					markers.data(), size);
			}
			else
			{
				requestedInfo.consoleWrite("Error, Mario kart markers file corrupted\n");
			}

		}

		size = 0;
		if (requestedInfo.getFileSizeBinary(PIKA_RESOURCES_PATH "/marioKart/coins.bin", size))
		{

			if (size % sizeof(glm::vec3) == 0 && size)
			{
				std::vector<glm::vec3> positions;
				positions.resize(size / sizeof(glm::vec3));

				requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "/marioKart/coins.bin",
					positions.data(), size);

				for (auto &p : positions)
				{
					coins.push_back(renderer.createEntity(coinModel, gl3d::Transform{p}, false));
					coinTimers.push_back(0);
				}
			}
			else
			{
				requestedInfo.consoleWrite("Error, Mario kart coins file corrupted\n");
			}

		}

		for (int i = 0; i < 4; i++)
		{
			player.wheels[i] = renderer.createEntity(wheelModel, {}, false);
		}

		if (markers.size() >= 2)
		{
			auto pos = markers[0].position;
			player.carPosition = pos;
		}
		else
		{
			return false;
		}
		//

		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
	#pragma region init stuff
		renderer2d.updateWindowMetrics(windowState.windowW, windowState.windowH);

		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
		renderer.camera.aspectRatio = (float)windowState.windowW / windowState.windowH; //todo do this in update
	#pragma endregion
		
		if (input.buttons[pika::Button::Escape].released())
		{
			::pika::pikaImgui::removeFocusToCurrentWindow();
		}

		auto getLeft = [&](MarioKartEditor::RoadMarker &m)
		{
			glm::vec3 cross = {1,0,0};
			cross = glm::vec3(glm::rotate(m.tilt, glm::vec3{0,0,1}) * glm::vec4(cross, 1));
			cross = glm::vec3(glm::rotate(m.angle, glm::vec3{0,1,0}) * glm::vec4(cross, 1));
			cross = glm::normalize(cross);
			return m.position - cross * m.size;
		};

		auto getRight = [&](MarioKartEditor::RoadMarker &m)
		{
			glm::vec3 cross = {1,0,0};
			cross = glm::vec3(glm::rotate(m.tilt, glm::vec3{0,0,1}) * glm::vec4(cross, 1));
			cross = glm::vec3(glm::rotate(m.angle, glm::vec3{0,1,0}) * glm::vec4(cross, 1));
			cross = glm::normalize(cross);
			return m.position + cross * m.size;
		};

	#pragma region spheres


		while (spheres.size() > markers.size() * 3)
		{
			renderer.deleteEntity(spheres.back());
			spheres.pop_back();
		}

		while (spheres.size() < markers.size() * 3)
		{
			spheres.push_back(renderer.createEntity(sphereModel, {}, false));
		}
		
		if (showGuides)
		{
			int currentSphere = 0;
			for (auto m : markers)
			{
				gl3d::MaterialValues mat = renderer.getEntityMeshMaterialValues(spheres[currentSphere], 0);
				//mat.kd.a = 0.5;

				if (currentMarker == currentSphere / 3)
				{
					mat.kd.r = 1;
					mat.kd.g = 0;
					mat.kd.b = 0;
				}
				else
				{
					mat.kd.r = 0;
					mat.kd.g = 1;
					mat.kd.b = 0;
				}

				gl3d::Transform t;
				t.position = m.position;

				t.scale = glm::vec3(0.2);

				renderer.setEntityMeshMaterialValues(spheres[currentSphere], 0, mat);
				renderer.setEntityVisible(spheres[currentSphere], true);
				renderer.setEntityTransform(spheres[currentSphere++], {t});
				t.scale = glm::vec3(0.1);

				t.position = getRight(m);
				renderer.setEntityMeshMaterialValues(spheres[currentSphere], 0, mat);
				renderer.setEntityVisible(spheres[currentSphere], true);
				renderer.setEntityTransform(spheres[currentSphere++], {t});

				t.position = getLeft(m);
				mat.kd.b = 1;
				renderer.setEntityMeshMaterialValues(spheres[currentSphere], 0, mat);
				renderer.setEntityVisible(spheres[currentSphere], true);
				renderer.setEntityTransform(spheres[currentSphere++], {t});


			}
		}
		else
		{
			for (auto &s : spheres)
			{
				renderer.setEntityVisible(s, false);
			}
		}
		

	#pragma endregion

	#pragma region coins

		spinTimer += input.deltaTime * 3.f;

		if (spinTimer > 2 * 3.141592)
		{
			spinTimer -= 2 * 3.141592;
		}

		int coinCount = 0;
		for (auto &c : coins)
		{
			auto t = renderer.getEntityTransform(c);
			t.rotation.y = spinTimer;
			renderer.setEntityTransform(c, t);

			if (coinTimers[coinCount] > 0)
			{
				coinTimers[coinCount] -= input.deltaTime;
				if (coinTimers[coinCount] <= 0)
				{
					coinTimers[coinCount] = 0;
					renderer.setEntityVisible(c, true);
				}
			}
			else
			{
				if (glm::distance(player.carPosition, t.position) < 1.f)
				{
					coinTimers[coinCount] = 10;
					renderer.setEntityVisible(c, false);
					score++;
				}
			}


			coinCount++;
		}

	#pragma endregion


	#pragma region car

		float tilt = 0;

		//input
		if (markers.size() >= 2 && gameplayData.gameplayPhaze == 1)
		{

			float moveForward = 0;
			if (input.buttons[pika::Button::Up].held() || (input.buttons[pika::Button::W].held() && !freeCamera))
			{
				moveForward = 1;
			}
			if (input.buttons[pika::Button::Down].held() || (input.buttons[pika::Button::S].held() && !freeCamera))
			{
				moveForward -= 1;
			}

			acceleration += moveForward * input.deltaTime * 3.f;

			acceleration = glm::clamp(acceleration, -10.f, 20.f);

			auto isLeft2 = [](glm::vec2 a, glm::vec2 b, glm::vec2 c)
			{
				return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) > 0;
			};

			auto isLeft = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c)
			{
				return isLeft2({a.x,a.z}, {b.x,b.z}, {c.x, c.z});
			};

			if (!moveForward)
			{
				if (acceleration > 0)
				{
					acceleration -= input.deltaTime;
					if (acceleration < 0) { acceleration = 0; }
				}
				else if (acceleration < 0)
				{
					acceleration += input.deltaTime;
					if (acceleration > 0) { acceleration = 0; }
				}
			}

			if (acceleration)
			{
				player.carPosition.y = 0;

				float leftRight = 0;
				if (input.buttons[pika::Button::Left].held() || (input.buttons[pika::Button::A].held() && !freeCamera))
				{
					leftRight = 1;
				}
				if (input.buttons[pika::Button::Right].held() || (input.buttons[pika::Button::D].held() && !freeCamera))
				{
					leftRight -= 1;
				}

				if (leftRight)
				{
					player.moveDirection =
						glm::rotate(input.deltaTime * leftRight * 2.f, glm::vec3{0,1,0}) * 
						glm::vec4(player.moveDirection, 1);

					player.moveDirection = glm::normalize(player.moveDirection);
				}


				//auto oldPos = carPosition;
				//carPosition += 
				auto move = player.moveDirection * input.deltaTime * acceleration;
				
				//glm::vec2 newDelta = markerDelta + glm::vec2(move.x, move.y);

				auto m1 = markers[player.carMarker];
				auto m2 = markers[(player.carMarker + 1) % markers.size()];

				if (!isLeft(getLeft(m1), getLeft(m2), player.carPosition + glm::vec3(move.x, 0, move.z)))
				{
					acceleration = -acceleration*0.2;
				}
				else if (isLeft(getRight(m1), getRight(m2), player.carPosition + glm::vec3(move.x, 0, move.z)))
				{
					acceleration = -acceleration * 0.2;
				}
				else
				{

					if (!isLeft(getLeft(m2), getRight(m2), player.carPosition + glm::vec3(move.x, 0, move.z)))
					{
						player.carMarker++;
						player.carMarker = player.carMarker % markers.size();
					}
					else if (isLeft(getLeft(m1), getRight(m1), player.carPosition + glm::vec3(move.x, 0, move.z)))
					{
						player.carMarker--;
						if (player.carMarker < 0) { player.carMarker = markers.size() - 1; }
					}

					player.carPosition += glm::vec3(move.x, 0, move.z);
				}
				

				//float dist1 = glm::distance(glm::vec2(m1.position.x, m1.position.z), 
				//	glm::vec2(carPosition.x, carPosition.z));
				//
				//float dist2 = glm::distance(glm::vec2(m2.position.x, m2.position.z),
				//	glm::vec2(carPosition.x, carPosition.z));



				//carPosition = markers[carMarker].position;
				//carPosition.x += newDelta.x;
				//carPosition.z += newDelta.y;

				//for (int i = 0; i < markers.size(); i++)
				//{
				//
				//	auto m1 = markers[i];
				//	auto m2 = markers[(i+1)%markers.size()];
				//	
				//
				//	//check point in box;
				//
				//	auto a1 = getLeft(m1);
				//	auto a2 = getRight(m1);
				//	auto a3 = getRight(m2);
				//	auto a4 = getLeft(m2);
				//
				//
				//
				//}



			}


			auto updateCar = [&](Pilot &pilot)
			{
				auto m1 = markers[pilot.carMarker];
				auto m2 = markers[(pilot.carMarker + 1) % markers.size()];

				{
					glm::vec3 roadLine = m2.position - m1.position;
					glm::vec3 p = pilot.carPosition - m1.position; p.y = 0;
					p.y = 0;
					roadLine.y = 0;

					auto projPoint = glm::dot(p, roadLine) / glm::length(roadLine);

					float l = glm::length(projPoint);

					float final = l / glm::length(roadLine);

					pilot.carPosition.y = glm::mix(m1.position.y, m2.position.y, glm::clamp(final, 0.f, 1.f));
					tilt = glm::mix(m1.tilt, m2.tilt, glm::clamp(final, 0.f, 1.f));
					//carPosition.y = m1.position.y;
				}

				glm::vec3 offset(0, 0.18, 0);

				float angle = std::atan2(pilot.moveDirection.z, -pilot.moveDirection.x) - glm::radians(90.f);

				gl3d::Transform t;
				t.position = pilot.carPosition + offset;
				t.rotation.y = angle;
				t.rotation.z = -tilt;
				t.scale = glm::vec3(0.30);
				renderer.setEntityTransform(pilot.car, t);
				renderer.setEntityTransform(pilot.character, t);

				static float wheelTimer = 0;
				wheelTimer += input.deltaTime * acceleration;

				t.scale *= glm::vec3(0.065);
				for (int i = 0; i < 4; i++)
				{
					//auto t2 = t;
					glm::vec3 pos[4] = {glm::vec3(8.2, 0, 11.0),
						glm::vec3(8.2, 0, -11.0),
						glm::vec3(-8.2, 0, 11.0),
						glm::vec3(-8.2, 0, -11.0),
					};

					auto mat = t.getTransformMatrix();
					mat = mat * glm::translate(pos[i]);
					auto t2 = gl3d::Transform{};
					t2.setFromMatrix(mat);

					t2.rotation.x = wheelTimer;


					renderer.setEntityTransform(pilot.wheels[i], t2);


				}
			};

			updateCar(player);
			
		}

			
		if (input.buttons[pika::Button::P].pressed())
		{
			freeCamera = !freeCamera;
		}
		if (input.buttons[pika::Button::O].pressed())
		{
			showGuides = !showGuides;
		}

		if (gameplayData.gameplayPhaze == 0)
		{
			gameplayData.animationTimer -= input.deltaTime;

			if (gameplayData.animationTimer <= 0)
			{
				gameplayData.gameplayPhaze = 1;
			}
			else
			{
				if (gameplayData.animationTimer > 6.f)
				{
					float normalized = glm::clamp((gameplayData.animationTimer-6.f) / 3.f, 0.f, 1.f);
					renderer.camera.position = glm::mix(glm::vec3(8.9, 18.26, -2), glm::vec3(0, 20, -10),
						1 - normalized);
					renderer.camera.viewDirection = glm::normalize(glm::vec3{-0.963, 0.0186, 0.266});
				}else if (gameplayData.animationTimer > 3.f)
				{
					float normalized = glm::clamp((gameplayData.animationTimer - 3.f) / 3.f, 0.f, 1.f);
					renderer.camera.position = glm::mix(glm::vec3(13, 18, -64), glm::vec3(35, 19, -55),
						1 - normalized);
					renderer.camera.viewDirection = glm::normalize(glm::vec3{0.980, -0.0136, 0.142});
				}
				else
				{
					float normalized = glm::clamp((gameplayData.animationTimer) / 3.f, 0.f, 1.f);
					renderer.camera.position = glm::mix(glm::vec3(38,20,32), glm::vec3(46,17,28),
						1 - normalized);
					renderer.camera.viewDirection = glm::normalize(glm::vec3{0.158, -0.161, -0.974});
				}

				
			}

			//8.9 18.26, -2    -> -3, 24, -11
			//view: -0.963, 0.0186, 0.266

			//13, 10, -64 ->  45, 19, -53
			//view: 0.975, -0.0905, 0.201
		}
		else
		{
			if (freeCamera)
			{
				editor.update(requestedInfo.requestedImguiIds, renderer, input, cameraSpeed, requestedInfo, {windowState.windowW,windowState.windowH});
			}
			else
			{
				renderer.camera.position = player.carPosition + glm::vec3(0, 1.5, 0) - player.moveDirection *
					(1.5f + glm::clamp(acceleration / 15.f, 0.f, 1.f) * 1.0f);
				renderer.camera.up = glm::vec3(glm::rotate(-tilt * 2.f, glm::vec3(0, 0, 1)) * glm::vec4(0, 1, 0, 1));
				renderer.camera.viewDirection = glm::normalize(player.moveDirection + glm::vec3(0, -0.4, 0));
			}
		}
	


	#pragma endregion
			


		renderer.render(input.deltaTime);
		glDisable(GL_DEPTH_TEST);

		renderer2d.renderText({100, 50}, std::to_string(score).c_str(), font, Colors_White);

		renderer2d.flush();


		return true;
	}

	void destruct(RequestedContainerInfo &requestedInfo) override
	{
		renderer.skyBox.clearTextures();
		renderer.clearAllRendererResources();
	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release