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

struct ThreeDEditor: public Container
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

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{

		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;
		
		renderer.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", requestedInfo.requestedFBO.fbo);
		
		const char *names[6] =
		{PIKA_RESOURCES_PATH "/skyBoxes/ocean/right.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/left.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/top.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/bottom.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/front.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/back.jpg"};
		
		//renderer.skyBox = renderer.loadSkyBox(names);
		//renderer.skyBox.color = {0.2,0.3,0.8};
		renderer.skyBox = renderer.atmosfericScattering({0,1,0}, {0.2,0.2,0.5}, {0.6,0.2,0.1}, {},
			false, 10);

		//helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "helmet/helmet.obj");
		model = renderer.loadModel(PIKA_RESOURCES_PATH "rave.glb", 0.5, gl3d::TextureLoadQuality::maxQuality);
		
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

		

		renderer.updateWindowMetrics(windowState.w, windowState.h);
		renderer.camera.aspectRatio = (float)windowState.w / windowState.h; //todo do this in update

			
		editor.update(requestedInfo.requestedImguiIds, renderer, input, 4, requestedInfo);


		renderer.render(input.deltaTime);
		glDisable(GL_DEPTH_TEST);


		return true;
	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release