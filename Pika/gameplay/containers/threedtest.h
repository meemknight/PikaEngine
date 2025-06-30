#pragma once

#include <gl2d/gl2d.h>
#include <gl3d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <engineLibraresSupport/engineGL3DSupport.h>


struct ThreeDTest: public Container
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
	gl3d::Model helmetModel;
	gl3d::Entity helmetEntity;

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
		renderer.skyBox.color = {0.4,0.4,0.4};


		helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "helmet/helmet.obj", gl3d::TextureLoadQuality::maxQuality, 1.f);
		//helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "/knight/uploads_files_1950170_Solus_the_knight.gltf", 1.f);

		
		renderer.createDirectionalLight({-1,-0.5,-0.2});


		gl3d::Transform t;
		//t.position = {400'000, 0, -4};
		t.rotation = {1.5, 0 , 0};

		helmetEntity = renderer.createEntity(helmetModel, t);
	


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

		renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
		renderer.camera.aspectRatio = (float)windowState.windowW / windowState.windowH; //todo do this in update
		
		static glm::dvec2 lastMousePos = {};
		pika::gl3d::fpsInput(renderer, input, 5, lastMousePos, requestedInfo, {windowState.windowW, windowState.windowH});

		if(0)
		{
			if (input.rMouse.held())
			{
				glm::dvec2 currentMousePos = {input.mouseX, input.mouseY};

				float speed = 0.8f;

				glm::vec2 delta = lastMousePos - currentMousePos;
				delta *= speed * input.deltaTime;

				renderer.camera.rotateCamera(delta);

				lastMousePos = currentMousePos;
			}
			else
			{
				lastMousePos = {input.mouseX, input.mouseY};
			}
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

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release