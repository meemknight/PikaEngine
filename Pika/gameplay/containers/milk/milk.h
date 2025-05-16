#pragma once

#include <gl2d/gl2d.h>
#include <gl3d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <engineLibraresSupport/engineGL3DSupport.h>
#include <safeSave/safeSave.h>
#include "physics.h"


struct Milk: public Container
{

	

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(100);

		info.requestImguiFbo = true;
		info.requestImguiIds = 1;

		return info;
	}

	gl3d::Renderer3D renderer;
	pika::gl3d::General3DEditor editor;
	gl3d::Material floorMaterial;

	gl3d::Model milkModel;
	gl3d::Entity milkEntity;


	Simulator simulator;

	struct Cube
	{
		glm::vec3 size = {1,1,1};
		gl3d::Model model;
		gl3d::Entity entity;
		int physicsID = 0;
	};

	std::vector<Cube> cubes;

	std::vector<Cube> physicsCubes;


	gl3d::Model createCubeModel(glm::vec3 size);

	int playerPhysics = 0;
	glm::vec3 playerSize = {0.5, 1.8, 0.5};


	void addNewCube(glm::vec3 size, glm::vec3 position, glm::vec3 rotation = {})
	{
		Cube c;

		gl3d::Transform t;
		t.position = position;
		t.rotation = rotation;

		c.size = size;
		c.model = createCubeModel(size);
		c.entity = renderer.createEntity(c.model, t);
		

		auto box = createBox(position, size);
		box.mass = INFINITY;

		int id = simulator.getIdAndIncrement();

		simulator.bodies.insert({id, box});
		c.physicsID = id;


		cubes.push_back(c);

	};

	void addNewCubePhysics(glm::vec3 size, glm::vec3 position, glm::vec3 rotation = {})
	{
		Cube c;

		gl3d::Transform t;
		t.position = position;
		t.rotation = rotation;

		c.size = size;
		c.model = createCubeModel(size);
		c.entity = renderer.createEntity(c.model, t);


		auto box = createBox(position, size);

		int id = simulator.getIdAndIncrement();

		simulator.bodies[id] = box;
		c.physicsID = id;


		physicsCubes.push_back(c);

	};

	void modifyCube(int index, glm::vec3 size, bool force = 0)
	{

		auto &c = cubes[index];

		if (c.size != size || force)
		{
			auto transform = renderer.getEntityTransform(c.entity);
			renderer.deleteEntity(c.entity);
			renderer.deleteModel(c.model);
			
			c.size = size;
			c.model = createCubeModel(size);
			c.entity = renderer.createEntity(c.model, transform);

			auto found = simulator.bodies.find(c.physicsID);
			if (found != simulator.bodies.end()) 
			{
				found->second.shape = size;
			}
			
		}


	};

	void deleteCube(int index)
	{
		if (index < 0 || index >= cubes.size()) { return; }

		auto &c = cubes[index];

		auto found = simulator.bodies.find(c.physicsID);
		if (found != simulator.bodies.end()) { simulator.bodies.erase(found); }
		
		renderer.deleteEntity(c.entity);
		renderer.deleteModel(c.model);

		cubes.erase(cubes.begin() + index);

	}

	void load(RequestedContainerInfo &requestedInfo)
	{
		sfs::SafeSafeKeyValueData loadedData;
		std::vector<char> binaryData;

		requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "milk/level.bin", binaryData);

		loadedData.loadFromFileData(binaryData.data(), binaryData.size());

		while (cubes.size())
		{
			deleteCube(cubes.size() - 1);
		}

		for (auto &e : loadedData.entries)
		{
			sfs::SafeSafeKeyValueData cube;

			if (loadedData.getKeyValueData(e.first, cube) == sfs::noError)
			{
				glm::vec3 size = {};
				glm::vec3 position = {};
				glm::vec3 rotation = {};

				cube.getVec3("size", size.x, size.y, size.z);
				cube.getVec3("position", position.x, position.y, position.z);
				cube.getVec3("rotation", rotation.x, rotation.y, rotation.z);

				addNewCube(size, position, rotation);
			}

		}
	}

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
	
		//glEnable(GL_DEBUG_OUTPUT);
		//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		//glDebugMessageCallback(gl3d::glDebugOutput, &renderer.errorReporter);
		//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);


		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;

		renderer.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", requestedInfo.requestedFBO.fbo);

		renderer.adaptiveResolution.useAdaptiveResolution = false;

		//renderer.skyBox = renderer.atmosfericScattering({0.2,1,0.3}, {0.9,0.1,0.1}, {0.4, 0.4, 0.8}, 0.8f); //todo a documentation
		//todo api for skybox stuff
		//renderer.skyBox.color = {0.2,0.3,0.9};

		//const char *names[6] =
		//{	PIKA_RESOURCES_PATH "/skyBoxes/ocean/right.jpg",
		//	PIKA_RESOURCES_PATH "/skyBoxes/ocean/left.jpg",
		//	PIKA_RESOURCES_PATH "/skyBoxes/ocean/top.jpg",
		//	PIKA_RESOURCES_PATH "/skyBoxes/ocean/bottom.jpg",
		//	PIKA_RESOURCES_PATH "/skyBoxes/ocean/front.jpg",
		//	PIKA_RESOURCES_PATH "/skyBoxes/ocean/back.jpg"};
		//
		//renderer.skyBox = renderer.loadSkyBox(names);
		//renderer.skyBox.color = {0.2,0.3,0.8};

		renderer.skyBox = renderer.loadHDRSkyBox(PIKA_RESOURCES_PATH "/skyBoxes/canary_wharf_2k.hdr");
		renderer.skyBox.color = {139.f / 255.f,139.f / 255.f,139.f / 255.f};
		floorMaterial = renderer.loadMaterial(PIKA_RESOURCES_PATH "milk/floor/floor.mtl", gl3d::TextureLoadQuality::maxQuality)[0];


		renderer.createPointLight({4.6,8.8,-8.5}, glm::vec3{169 / 255.f}, 27.150, 1.2);

		auto l = renderer.createPointLight({36,12.6,-10.4}, glm::vec3{237.f, 37.f, 37.f} / 255.f, 20, 1);
		renderer.setPointLightShadows(l, false);

		renderer.createPointLight({66.1,10.2,-9.3}, glm::vec3{57.f, 221.f, 215.f} / 255.f, 20, 1);



		//helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "helmet/helmet.obj", gl3d::TextureLoadQuality::maxQuality, 1.f);
		//helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "/knight/uploads_files_1950170_Solus_the_knight.gltf", 1.f);


		//renderer.createDirectionalLight({-1,-0.5,-0.2});

		milkModel = renderer.loadModel(PIKA_RESOURCES_PATH "milk/milk.glb", gl3d::TextureLoadQuality::maxQuality, 1);
		milkEntity = renderer.createEntity(milkModel, {{66.9,4.9,-9.5}}, false);

		//optional make milk bottle shiny
		//gl3d::MaterialValues mat;
		//mat.roughness = 0.1;
		//renderer.setEntityMeshMaterialValues(milkEntity, 0, mat);
		//renderer.setEntityMeshMaterialValues(milkEntity, 1, mat);

		load(requestedInfo);

		{
			auto box = createBox({0, 2, 0}, playerSize);
			box.mass = 50;
			box.bouncyness = 0;

			playerPhysics = simulator.getIdAndIncrement();

			simulator.bodies.insert({playerPhysics, box});
		}
		

		return true;
	}

	int selectedCube = -1;

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
	
	#pragma region frame start stuff
		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
		renderer.camera.aspectRatio = (float)windowState.windowW / windowState.windowH; //todo do this in update

		
		editor.update(requestedInfo.requestedImguiIds, renderer, input, 0, requestedInfo, {windowState.windowW,windowState.windowH});

		auto &playerObject = simulator.bodies[playerPhysics];

		{
			float speed = 10;
			glm::vec3 dir = {};
			if (input.buttons[pika::Button::W].held())
			{
				dir.z -= speed * input.deltaTime;
			}
			if (input.buttons[pika::Button::S].held())
			{
				dir.z += speed * input.deltaTime;
			}

			if (input.buttons[pika::Button::A].held())
			{
				dir.x -= speed * input.deltaTime;
			}
			if (input.buttons[pika::Button::D].held())
			{
				dir.x += speed * input.deltaTime;
			}

			//if (input.buttons[pika::Button::Q].held())
			//{
			//	dir.y -= speed * input.deltaTime;
			//}
			//if (input.buttons[pika::Button::E].held())
			//{
			//	dir.y += speed * input.deltaTime;
			//}

			if (dir.x != 0 || dir.y != 0 || dir.z != 0)
			{
				glm::vec3 viewDirection = glm::normalize(renderer.camera.viewDirection);

				//forward
				float forward = -dir.z;
				float leftRight = dir.x;
				float upDown = dir.y;

				glm::vec3 move = {};

				move += glm::vec3(0, 1, 0) * upDown;
				move += glm::normalize(glm::cross(viewDirection, glm::vec3(0, 1, 0))) * leftRight;
				move += viewDirection * forward;

				playerObject.velocity += move;
			}

			if (input.buttons[pika::Button::Space].pressed())
			{
				playerObject.velocity.y += 10.f;
			}

		}

		renderer.camera.position = playerObject.position + glm::vec3(0, 0.7, 0);


		if (input.buttons[pika::Button::Escape].released())
		{
			::pika::pikaImgui::removeFocusToCurrentWindow();
		}

		
		{
			static glm::dvec2 lastMousePos = {};
			if (input.rMouse.held())
			{
				glm::dvec2 currentMousePos = {input.mouseX, input.mouseY};

				float speed = 0.8f;

				glm::vec2 delta = lastMousePos - currentMousePos;
				delta *= speed * 0.01;

				renderer.camera.rotateCamera(delta);

				lastMousePos = currentMousePos;
			}
			else
			{
				lastMousePos = {input.mouseX, input.mouseY};
			}
		}
	#pragma endregion

		ImGui::PushID(requestedInfo.requestedImguiIds);
		if (ImGui::Begin("General3DEditor"))
		{
			ImGui::Separator();
			ImGui::Text("Level data");
			if (ImGui::Button("Save"))
			{
				sfs::SafeSafeKeyValueData rez;

				for (int i = 0; i < cubes.size(); i++)
				{
					sfs::SafeSafeKeyValueData cube;
					
					glm::vec3 size = cubes[i].size;
					auto transform = renderer.getEntityTransform(cubes[i].entity);
					glm::vec3 position = transform.position;
					glm::vec3 rotation = transform.rotation;

					cube.setVec3("size", size.x, size.y, size.z);
					cube.setVec3("position", position.x, position.y, position.z);
					cube.setVec3("rotation", rotation.x, rotation.y, rotation.z);

					rez.setKeyValueData(std::to_string(i), cube);
				}

				auto finalData = rez.formatIntoFileDataBinary();

				requestedInfo.writeEntireFileBinary(PIKA_RESOURCES_PATH "milk/level.bin",
					finalData.data(), finalData.size());
			}
			
			ImGui::SameLine();

			if (ImGui::Button("Load"))
			{
				load(requestedInfo);
			}

			ImGui::Separator();

			if (ImGui::Button("Recreate"))
			{
				for (int i = 0; i < cubes.size(); i++)
				{
					modifyCube(i, cubes[i].size, true);
				}
			}

			if (ImGui::Button("New"))
			{
				addNewCube({1,1,1}, {0,0,0});
				selectedCube = cubes.size() - 1;
			}

			if (ImGui::Button("New Physics"))
			{
				addNewCubePhysics({1,1,1}, {0,10,0});
			}

			if (cubes.size())
			{
				ImGui::SliderInt("Select Cube:", &selectedCube, -1, cubes.size() - 1);

				if (selectedCube >= 0)
				{
					auto s = cubes[selectedCube].size;

					auto transform = renderer.getEntityTransform(cubes[selectedCube].entity);

					ImGui::DragFloat3("Size", &s[0], 0.1);
					ImGui::DragFloat3("Position", &transform.position[0], 0.1);
					ImGui::DragFloat3("Rotation", &transform.rotation[0], 0.1);

					renderer.setEntityTransform(cubes[selectedCube].entity, transform);

					modifyCube(selectedCube, s);

					auto found = simulator.bodies.find(cubes[selectedCube].physicsID);
					if (found != simulator.bodies.end())
					{
						found->second.position = transform.position;
					}

					if (ImGui::Button("Delte"))
					{
						deleteCube(selectedCube);
					}
				}

			}
			else
			{
				selectedCube = -1;
			}



		}
		ImGui::End();
		ImGui::PopID();
		

		simulator.update(input.deltaTime);


		for (auto &c : physicsCubes)
		{
			auto found = simulator.bodies.find(c.physicsID);
			if (found != simulator.bodies.end())
			{
				auto transform = renderer.getEntityTransform(c.entity);
				transform.position = found->second.position;
				renderer.setEntityTransform(c.entity, transform);
			}
		}

		for (auto &c : cubes)
		{
			auto found = simulator.bodies.find(c.physicsID);
			if (found != simulator.bodies.end())
			{
				auto transform = renderer.getEntityTransform(c.entity);
				transform.position = found->second.position;
				renderer.setEntityTransform(c.entity, transform);
			}
		}

		{
			static float timer = 0;
			static glm::vec3 posOriginal = {};

			auto t = renderer.getEntityTransform(milkEntity);

			if (posOriginal == glm::vec3()) { posOriginal = t.position; }

			t.rotation.x = 0.8;
			t.rotation.y = timer * -0.5;
			t.position.y = posOriginal.y + sin(timer * -0.6f);

			timer += input.deltaTime * 1.4;

			renderer.setEntityTransform(milkEntity, t);
		}


		renderer.render(input.deltaTime);


		glDisable(GL_DEPTH_TEST);


		//if (!ImGui::Begin("Test window"))
		//{
		//	ImGui::End();
		//	return;
		//}
		//
		//auto t = renderer.getEntityMeshMaterialTextures(helmetEntity, 0);
		//
		//ImGui::Image((void*)renderer.getTextureOpenglId(t.albedoTexture), {200, 200});
		//
		//ImGui::End();

		return true;
	}


	void destruct(RequestedContainerInfo &requestedInfo)
	{

		this->renderer.skyBox.clearTextures();
		this->renderer.colorCorrectionTexture().clear();
		this->renderer.clearAllRendererResources();


	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release