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

	std::vector<RoadMarker> markers;
	int currentMarker = 0;

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
	bool first = 1;

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
		worldModel = renderer.loadModel("C:/Users/meemk/Desktop/map2/N64 Royal Raceway.obj",
			gl3d::TextureLoadQuality::maxQuality, 1);

		
		//todo error by gl3d when creating an entity with no model loaded
		sphereModel = renderer.loadModel(PIKA_RESOURCES_PATH "/marioKart/sphere.obj",
			gl3d::TextureLoadQuality::maxQuality, 1);

		worldEntity = renderer.createEntity(worldModel, {});


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

			glm::vec3 cross = {1,0,0};
			cross = glm::vec3(glm::rotate(m.tilt, glm::vec3{0,0,1}) * glm::vec4(cross, 1));
			cross = glm::vec3(glm::rotate(m.angle, glm::vec3{0,1,0}) * glm::vec4(cross, 1));
			cross = glm::normalize(cross);

			t.scale = glm::vec3(0.1);

			t.position = m.position + cross * m.size;
			renderer.setEntityMeshMaterialValues(spheres[currentSphere], 0, mat);
			renderer.setEntityTransform(spheres[currentSphere++], {t});

			t.position = m.position - cross * m.size;
			renderer.setEntityMeshMaterialValues(spheres[currentSphere], 0, mat);
			renderer.setEntityTransform(spheres[currentSphere++], {t});


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