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

struct MarioKartEditor: public Container
{

	struct RoadMarker
	{
		glm::vec3 position = {};
		float size = 3.f;
		float angle = 0.f;
		float tilt = 0.f;
		float unused1 = 0;
		float unused2 = 0;
		int unused3 = 0;
		int unused4 = 0;
		unsigned char isAir = 0;
	};

	glm::vec3 carPosition = {};
	glm::vec3 moveDirection = glm::normalize(glm::vec3{0,0,-1});
	int carMarker = 0;

	std::vector<RoadMarker> markers;
	int currentMarker = 0;
	glm::vec2 markerDelta = {};

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
	gl3d::Entity worldEntity;
	gl3d::Model carModel;
	gl3d::Entity carEntity;
	bool first = 1;
	float carPos = 0;


	std::vector<gl3d::Entity> spheres;

	pika::gl3d::General3DEditor editor;

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{

		//todo add hbao

		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;
		renderer.frustumCulling = false;

		renderer.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", requestedInfo.requestedFBO.fbo);
		

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

		renderer.createDirectionalLight({-0.155,-0.907,0.391}, glm::vec3{270.f/255.f});
		renderer.getDirectionalShadowCascadesFrustumSplit(0) = 0.10;
		renderer.getDirectionalShadowCascadesFrustumSplit(1) = 0.150;
		renderer.getDirectionalShadowCascadesFrustumSplit(2) = 0.640;
		renderer.camera.farPlane = 250;

		//helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "helmet/helmet.obj");
		//worldModel = renderer.loadModel("C:/Users/meemk/Desktop/map2/N64 Royal Raceway.obj",
		//	gl3d::TextureLoadQuality::maxQuality, 1);
		worldModel = renderer.loadModel("C:/Users/meemk/Desktop/map2/MarioRaceEnv.obj",
			gl3d::TextureLoadQuality::maxQuality, 1);
		
		carModel = renderer.loadModel("C:/Users/meemk/Desktop/car/Standard Kart.obj",
			gl3d::TextureLoadQuality::maxQuality, 1.f);

		//todo error by gl3d when creating an entity with no model loaded
		sphereModel = renderer.loadModel(PIKA_RESOURCES_PATH "/marioKart/sphere.obj",
			gl3d::TextureLoadQuality::maxQuality, 1);

		worldEntity = renderer.createEntity(worldModel, {});


		carEntity = renderer.createEntity(carModel);

		size_t size = 0;
		if (requestedInfo.getFileSizeBinary(PIKA_RESOURCES_PATH "/marioKart/markers.bin", size))
		{

			if (size % sizeof(RoadMarker) == 0 && size)
			{
				markers.resize(size / sizeof(RoadMarker));

				requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "/marioKart/markers.bin",
					markers.data(), size);
			}
			else
			{
				requestedInfo.consoleWrite("Error, Mario kart markers file corrupted\n");
			}

		}

		if (markers.size() >= 2)
		{
			auto pos = markers[0].position;
			carPosition = pos;
		}
		//

		return true;
	}

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

		renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
		renderer.camera.aspectRatio = (float)windowState.windowW / windowState.windowH; //todo do this in update
	#pragma endregion
		
		if (input.buttons[pika::Button::Escape].released())
		{
			::pika::pikaImgui::removeFocusToCurrentWindow();
		}

		auto getLeft = [&](RoadMarker &m)
		{
			glm::vec3 cross = {1,0,0};
			cross = glm::vec3(glm::rotate(m.tilt, glm::vec3{0,0,1}) * glm::vec4(cross, 1));
			cross = glm::vec3(glm::rotate(m.angle, glm::vec3{0,1,0}) * glm::vec4(cross, 1));
			cross = glm::normalize(cross);
			return m.position - cross * m.size;
		};

		auto getRight = [&](RoadMarker &m)
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
			renderer.setEntityTransform(spheres[currentSphere++], {t});

			t.scale = glm::vec3(0.1);

			t.position = getRight(m);
			renderer.setEntityMeshMaterialValues(spheres[currentSphere], 0, mat);
			renderer.setEntityTransform(spheres[currentSphere++], {t});

			t.position = getLeft(m);
			mat.kd.b = 1;
			renderer.setEntityMeshMaterialValues(spheres[currentSphere], 0, mat);
			renderer.setEntityTransform(spheres[currentSphere++], {t});


		}

	#pragma endregion

	#pragma region car

		if (markers.size() >= 2)
		{
			
			float moveForward = 0;
			if (input.buttons[pika::Button::Up].held())
			{
				moveForward = 1;
			}
			if (input.buttons[pika::Button::Down].held())
			{
				moveForward -= 1;
			}

			auto isLeft2 = [](glm::vec2 a, glm::vec2 b, glm::vec2 c)
			{
				return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) > 0;
			};

			auto isLeft = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c)
			{
				return isLeft2({a.x,a.z}, {b.x,b.z}, {c.x, c.z});
			};

			if (moveForward)
			{
				carPosition.y = 0;

				float leftRight = 0;
				if (input.buttons[pika::Button::Left].held())
				{
					leftRight = 1;
				}
				if (input.buttons[pika::Button::Right].held())
				{
					leftRight -= 1;
				}

				if (leftRight)
				{
					moveDirection = 
						glm::rotate(input.deltaTime * leftRight, glm::vec3{0,1,0}) * glm::vec4(moveDirection, 1);

					moveDirection = glm::normalize(moveDirection);
				}



				//auto oldPos = carPosition;
				//carPosition += 
				auto move = moveDirection * input.deltaTime * moveForward * 2.f;
				
				//glm::vec2 newDelta = markerDelta + glm::vec2(move.x, move.y);

				auto m1 = markers[carMarker];
				auto m2 = markers[(carMarker + 1)%markers.size()];

				if (!isLeft(getLeft(m2), getRight(m2), carPosition + glm::vec3(move.x, 0, move.z)))
				{
					carMarker++;
					carMarker = carMarker % markers.size();
				}
				else if (isLeft(getLeft(m1), getRight(m1), carPosition + glm::vec3(move.x, 0, move.z)))
				{
					carMarker--;
					if (carMarker < 0) { carMarker = markers.size() - 1; }
				}

				if (!isLeft(getLeft(m1), getLeft(m2), carPosition + glm::vec3(move.x, 0, move.z)))
				{

				}
				else if (isLeft(getRight(m1), getRight(m2), carPosition + glm::vec3(move.x, 0, move.z)))
				{

				}
				else
				{
					carPosition += glm::vec3(move.x, 0, move.z);
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

			auto m1 = markers[carMarker];
			auto m2 = markers[(carMarker + 1) % markers.size()];

			float tilt = 0;
			{
				glm::vec3 roadLine = m2.position - m1.position;
				glm::vec3 p = carPosition - m1.position; p.y = 0;
				p.y = 0;
				roadLine.y = 0;

				auto projPoint = glm::dot(p, roadLine) / glm::length(roadLine);

				float l = glm::length(projPoint);

				float final = l / glm::length(roadLine);

				carPosition.y = glm::mix(m1.position.y, m2.position.y, glm::clamp(final, 0.f, 1.f));
				tilt = glm::mix(m1.tilt, m2.tilt, glm::clamp(final, 0.f, 1.f));
				//carPosition.y = m1.position.y;
			}

			glm::vec3 offset(0, 0.4, 0);

			float angle = std::atan2(moveDirection.z, -moveDirection.x) - glm::radians(90.f);

			gl3d::Transform t;
			t.position = carPosition + offset;
			t.rotation.y = angle;
			t.rotation.z = -tilt;
			renderer.setEntityTransform(carEntity, t);
		}


	#pragma endregion
			


		ImGui::PushID(requestedInfo.requestedImguiIds);
		if (ImGui::Begin("General3DEditor"))
		{
			if (ImGui::CollapsingHeader("Move", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::DragFloat("Camera Speed", &cameraSpeed, 1, 0, 250);

				ImGui::DragFloat3("Camera Position", &renderer.camera.position[0]);

			}
		}
		ImGui::End();
		ImGui::PopID();

		editor.update(requestedInfo.requestedImguiIds, renderer, input, cameraSpeed, requestedInfo, {windowState.windowW,windowState.windowH});


		ImGui::PushID(requestedInfo.requestedImguiIds);
		if (ImGui::Begin("General3DEditor"))
		{
			if (ImGui::CollapsingHeader("Markers", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{

				ImGui::Text("Markers: %d", int(markers.size()));

				if (!markers.empty())
				{
					ImGui::InputInt("CurrentMarker: ", &currentMarker);

					currentMarker = std::clamp(currentMarker, 0, int(markers.size() - 1));

					RoadMarker &m = markers[currentMarker];

					ImGui::Separator();

					ImGui::DragFloat3("Marker position: ", &m.position[0], 0.02);
					ImGui::DragFloat("Size", &m.size, 0.02);
					ImGui::DragFloat("Angle", &m.angle, 0.002);
					ImGui::DragFloat("Tilt", &m.tilt, 0.002);
					bool isAir = m.isAir;
					ImGui::Checkbox("Air", &isAir);
					m.isAir = isAir;

					ImGui::Separator();
				}

				if (ImGui::Button("Add Marker"))
				{
					RoadMarker marker;
					marker.position = renderer.camera.position;

					markers.push_back(marker);
					currentMarker = markers.size() - 1;
				}

				if (ImGui::Button("Remove Current Marker"))
				{
					markers.erase(markers.begin() + currentMarker);
				}


				ImGui::Separator();


				if (ImGui::Button("Save"))
				{
					requestedInfo.writeEntireFileBinary(PIKA_RESOURCES_PATH "/marioKart/markers.bin",
						markers.data(), markers.size() * sizeof(RoadMarker));
				}


			}

			if (ImGui::CollapsingHeader("Car", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Text("Current marker: %d", carMarker);
			}
		
		}
		ImGui::End();


		ImGui::PopID();



		renderer.render(input.deltaTime);
		glDisable(GL_DEPTH_TEST);


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