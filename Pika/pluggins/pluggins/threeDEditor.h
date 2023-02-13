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

		info.requestImguiFbo = true;
		info.requestImguiIds = 1;

		return info;
	}

	ImGui::FileBrowser fileBrowserSkyBox;

	gl3d::Renderer3D renderer;
	gl3d::Model model;
	gl3d::Entity entity;

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{


		renderer.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer.fileOpener.userData = &requestedInfo;
		renderer.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer.fileOpener.fileExistsCallback = defaultFileExistsCustom;
		
		renderer.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", requestedInfo.requestedFBO.fbo);
		
		//renderer.skyBox = renderer.atmosfericScattering({0.2,1,0.3}, {0.9,0.1,0.1}, {0.4, 0.4, 0.8}, 0.8f); //todo a documentation
		//todo api for skybox stuff
		//renderer.skyBox.color = {0.2,0.3,0.9};
		
		const char *names[6] =
		{PIKA_RESOURCES_PATH "/skyBoxes/ocean/right.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/left.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/top.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/bottom.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/front.jpg",
			PIKA_RESOURCES_PATH "/skyBoxes/ocean/back.jpg"};
		
		renderer.skyBox = renderer.loadSkyBox(names);
		//renderer.skyBox.color = {0.2,0.3,0.8};
		
		//helmetModel = renderer.loadModel(PIKA_RESOURCES_PATH "helmet/helmet.obj");
		model = renderer.loadModel(PIKA_RESOURCES_PATH "rave.glb", 0.5);
		
		gl3d::Transform t;
		t.position = {0, -1, -4};
		//t.rotation = {1.5, 0 , 0};
		
		entity = renderer.createEntity(model, t);

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

		if(ImGui::Begin("Settings"))
		{
		
			if (ImGui::CollapsingHeader("Basic settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				pika::gl3d::generalSettingsWindow(requestedInfo.requestedImguiIds, renderer);
			}

			if (ImGui::CollapsingHeader("fxaa settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				pika::gl3d::fxaaSettingsWindow(requestedInfo.requestedImguiIds, renderer);
			}

			if (ImGui::CollapsingHeader("ssao settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				pika::gl3d::ssaoSettingsWindow(requestedInfo.requestedImguiIds, renderer);
			}

			if (ImGui::CollapsingHeader("ssr settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				pika::gl3d::ssrSettingsWindow(requestedInfo.requestedImguiIds, renderer);
			}

			if (ImGui::CollapsingHeader("bloom settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				pika::gl3d::bloomSettingsWindow(requestedInfo.requestedImguiIds, renderer);
			}

			if (ImGui::CollapsingHeader("chromatic aberation settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				pika::gl3d::chromaticAberationSettingsWindow(requestedInfo.requestedImguiIds, renderer);
			}
			
			if (ImGui::CollapsingHeader("lights settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{
				pika::gl3d::lightEditorSettingsWindow(requestedInfo.requestedImguiIds, renderer);
			}

			if (ImGui::CollapsingHeader("Sky box", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
			{

				if (ImGui::Button("delete skybox"))
				{
					renderer.skyBox.clearTextures();
				}

				if (ImGui::Button("select new skybox"))
				{
					fileBrowserSkyBox.SetTitle("Sellect map");
					fileBrowserSkyBox.SetPwd(PIKA_RESOURCES_PATH);
					fileBrowserSkyBox.SetTypeFilters({".hdr", ".png"});
					fileBrowserSkyBox.Open();
				}

				ImGui::ColorEdit3("Global Ambient color", &renderer.skyBox.color[0]);

			}
			
			fileBrowserSkyBox.Display();

			if (fileBrowserSkyBox.HasSelected())
			{
				if (fileBrowserSkyBox.GetSelected().extension() == ".hdr")
				{
					//todo api to log errors
					renderer.skyBox.clearTextures();
					renderer.skyBox = renderer.loadHDRSkyBox(fileBrowserSkyBox.GetSelected().string().c_str());
				}
				else if (fileBrowserSkyBox.GetSelected().extension() == ".png")
				{
					renderer.skyBox.clearTextures();
					renderer.skyBox = renderer.loadSkyBox(fileBrowserSkyBox.GetSelected().string().c_str());
				}

				fileBrowserSkyBox.ClearSelected();
				fileBrowserSkyBox.Close();
			}

		}
		ImGui::End();


	#pragma region input

	{
		float speed = 4;
		glm::vec3 dir = {};
		if (GetAsyncKeyState('W'))
		{
			dir.z -= speed * input.deltaTime;
		}
		if (GetAsyncKeyState('S'))
		{
			dir.z += speed * input.deltaTime;
		}

		if (GetAsyncKeyState('A'))
		{
			dir.x -= speed * input.deltaTime;
		}
		if (GetAsyncKeyState('D'))
		{
			dir.x += speed * input.deltaTime;
		}

		if (GetAsyncKeyState('Q'))
		{
			dir.y -= speed * input.deltaTime;
		}
		if (GetAsyncKeyState('E'))
		{
			dir.y += speed * input.deltaTime;
		}

		renderer.camera.moveFPS(dir);
	}

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

	#pragma endregion

		if (input.buttons[pika::Button::P].pressed())
		{
			renderer.setEntityAnimate(entity, true);
		}

		renderer.render(input.deltaTime);
		glDisable(GL_DEPTH_TEST);


		return true;
	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release