#pragma once

#include <gl2d/gl2d.h>
#include <gl3d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>

void inline errorCallbackCustom(std::string err, void *userData)
{
	RequestedContainerInfo *data = (RequestedContainerInfo*)userData;

	data->consoleWrite(err.c_str());
}

std::string inline readEntireFileCustom(const char *fileName, bool &couldNotOpen, void *userData)
{
	RequestedContainerInfo *data = (RequestedContainerInfo *)userData;
	couldNotOpen = false;

	size_t size = 0;
	if (!data->getFileSize(fileName, size))
	{
		couldNotOpen = true;
		return "";
	}

	std::string buffer;
	buffer.resize(size+1);

	if (!data->readEntireFile(fileName, &buffer.at(0), size))
	{
		couldNotOpen = true;
		return "";
	}

	return buffer;
}

std::vector<char> inline readEntireFileBinaryCustom(const char *fileName, bool &couldNotOpen, void *userData)
{
	RequestedContainerInfo *data = (RequestedContainerInfo *)userData;
	couldNotOpen = false;

	size_t size = 0;
	if (!data->getFileSizeBinary(fileName, size))
	{
		couldNotOpen = true;
		return {};
	}

	std::vector<char> buffer;
	buffer.resize(size + 1, 0);

	if (!data->readEntireFileBinary(fileName, &buffer.at(0), size))
	{
		couldNotOpen = true;
		return {};
	}

	return buffer;
}

bool inline defaultFileExistsCustom(const char *fileName, void *userData)
{
	RequestedContainerInfo *data = (RequestedContainerInfo *)userData;
	size_t s=0;
	return data->getFileSizeBinary(fileName, s);
}


struct ThreeDTest: public Container
{

	

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(20);

		info.requestImguiFbo = true;


		return info;
	}

	gl3d::Renderer3D renderer;
	gl3d::Model helmetModel;
	gl3d::Entity helmetEntity;

	void create(RequestedContainerInfo &requestedInfo)
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

		renderer.init(1, 1, requestedInfo.requestedFBO.fbo, PIKA_RESOURCES_PATH "BRDFintegrationMap.png");
		//renderer.skyBox = renderer.atmosfericScattering({0.2,1,0.3}, {0.9,0.1,0.1}, {0.4, 0.4, 0.8}, 0.8f); //todo a documentation
		//todo api for skybox stuff
		//renderer.skyBox.color = {0.2,0.3,0.9};

		const char *names[6] =
		{	PIKA_RESOURCES_PATH "/skyBoxes/ocean/right.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/left.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/top.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/bottom.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/front.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/back.jpg"};

		renderer.skyBox = renderer.loadSkyBox(names, requestedInfo.requestedFBO.fbo);
		//renderer.skyBox.color = {0.2,0.3,0.8};

		helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "helmet/helmet.obj", requestedInfo.requestedFBO.fbo);
		
		gl3d::Transform t;
		t.position = {0, 0, -3};
		t.rotation = {1.5, 0 , 0};

		helmetEntity = renderer.createEntity(helmetModel, t);

	}

	void update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
		//glDebugMessageCallback(gl3d::glDebugOutput, &renderer.errorReporter);
		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		renderer.updateWindowMetrics(windowState.w, windowState.h);
		renderer.camera.aspectRatio = (float)windowState.w / windowState.h; //todo do this in update
		
		{
			static glm::dvec2 lastMousePos = {};
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
		

		renderer.render(input.deltaTime, requestedInfo.requestedFBO.fbo);


		glDisable(GL_DEPTH_TEST);
	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release