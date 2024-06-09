#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <engineLibraresSupport/engineGL3DSupport.h>

struct Bezie : public Container
{

	gl2d::Renderer2D renderer;
	gl3d::Renderer3D renderer3D;
	pika::gl3d::General3DEditor editor;

	gl3d::Model sphere;
	gl3d::Entity sphereEntity;


	gl3d::Entity spheres[100];
	gl3d::Entity controlPoints[4];


	glm::vec3 point1 = {4.5,0,0};
	glm::vec3 point2 = {5.7,1.8,4};
	glm::vec3 point3 = {-0.3,7.8,0.1};
	glm::vec3 point4 = {0.7,10,4};

	unsigned char graph[128 * 128 * 4] = {};
	glm::vec3 pg1 = {0,0,0};
	glm::vec3 pg2 = {0.1,0.3,0};
	glm::vec3 pg3 = {0.9,0.2,0};
	glm::vec3 pg4 = {1,1,0};

	gl2d::Texture t;

	float anim = 0;
	bool animRunning = 0;

	glm::vec3 bezie(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, float t)
	{
		auto rez = glm::pow(1.f - t, 3.f) * p1 + 3.f * t * pow(1 - t, 2.f) * p2 + 3.f * t * t * (1.f - t) * p3 + t * t * t * p4;

		return rez;
	};

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(1000);

		info.requestImguiFbo = true; //todo this should not affect the compatibility of input recording

		//info.openOnApplicationStartup = true;


		return info;
	}

	bool freeCamera = true;

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		renderer.create(requestedInfo.requestedFBO.fbo);
		//pika::initShortcutApi();



		renderer3D.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer3D.fileOpener.userData = &requestedInfo;
		renderer3D.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer3D.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer3D.fileOpener.fileExistsCallback = defaultFileExistsCustom;

		renderer3D.init(1, 1, PIKA_RESOURCES_PATH "BRDFintegrationMap.png", requestedInfo.requestedFBO.fbo);

		renderer3D.skyBox = renderer3D.loadSkyBox(PIKA_RESOURCES_PATH "/skyBoxes/skybox.png");
		
		sphere = renderer3D.loadModel(PIKA_RESOURCES_PATH "/marioKart/sphere.obj", gl3d::maxQuality, 0.5f);
		sphereEntity = renderer3D.createEntity(sphere, {}, false, true, false);

		auto mat = renderer3D.getEntityMeshMaterialValues(sphereEntity, 0);
		mat.emmisive = 0.1;
		mat.kd.a = 0.8;
		renderer3D.setEntityMeshMaterialValues(sphereEntity, 0, mat);


		for (int i = 0; i < 100; i++)
		{

			gl3d::Transform t;
			t.scale = glm::vec3(0.1f);
			spheres[i] = renderer3D.createEntity(sphere, t, false, true, false);
			
			auto mat = renderer3D.getEntityMeshMaterialValues(spheres[i], 0);
			mat.emmisive = 0.6;
			mat.kd = glm::vec4{1,0,0, 0.2};
			renderer3D.setEntityMeshMaterialValues(spheres[i], 0, mat);
		}

		glm::vec4 colors[4] = {glm::vec4(0,0,1,0.4), glm::vec4(1,1,0,0.4), glm::vec4(0,1,1,0.4), glm::vec4(1,0,1,0.4)};
		for (int i = 0; i < 4; i++)
		{
			gl3d::Transform t;
			t.scale = glm::vec3(0.2f);
			controlPoints[i] = renderer3D.createEntity(sphere, t, false, true, false);

			auto mat = renderer3D.getEntityMeshMaterialValues(controlPoints[i], 0);
			mat.emmisive = 0.7;
			mat.kd = colors[i];
			renderer3D.setEntityMeshMaterialValues(controlPoints[i], 0, mat);

		}
		
		t.create1PxSquare(0);
		t.bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
		//todo keep window on top stuff
		renderer3D.setErrorCallback(&errorCallbackCustom, &requestedInfo);
		renderer3D.fileOpener.userData = &requestedInfo;
		renderer3D.fileOpener.readEntireFileBinaryCallback = readEntireFileBinaryCustom;
		renderer3D.fileOpener.readEntireFileCallback = readEntireFileCustom;
		renderer3D.fileOpener.fileExistsCallback = defaultFileExistsCustom;
		renderer3D.frameBuffer = requestedInfo.requestedFBO.fbo;
		renderer3D.updateWindowMetrics(windowState.windowW, windowState.windowH);
		renderer3D.camera.aspectRatio = (float)(windowState.windowW) / windowState.windowH;

		renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);

		glClear(GL_COLOR_BUFFER_BIT);


		if (animRunning)
		{
			anim += input.deltaTime / 2.f;

			if (anim >= 1)
			{
				anim = 1;
				animRunning = 0;
			}

		}

		if (input.buttons[pika::Button::Escape].released())
		{
			::pika::pikaImgui::removeFocusToCurrentWindow();
			requestedInfo.setNormalCursor();
			//freeCamera = false;
		}

		if (!input.hasFocus)
		{
			requestedInfo.setNormalCursor();
		}

		//if (input.buttons[pika::Button::F].released())
		//{
		//	freeCamera = !freeCamera;
		//}


		if (input.hasFocus)
		{
			editor.update(requestedInfo.requestedImguiIds, renderer3D, input,
				4, requestedInfo, {windowState.windowW,windowState.windowH});
		}

		if (input.buttons[pika::Button::Space].pressed())
		{
			animRunning = 1;
			anim = 0;
		}

		//randez curba 3D
		for (int i = 0; i < 100; i++)
		{
			auto t = renderer3D.getEntityTransform(spheres[i]);
			t.position = bezie(point1, point2, point3, point4, i / 100.f);
			renderer3D.setEntityTransform(spheres[i], t);
		}

		glm::vec2 graphPoint = {};

		//punctele de control 3D
		{
			auto t = renderer3D.getEntityTransform(controlPoints[0]);
			t.position = point1;
			renderer3D.setEntityTransform(controlPoints[0], t);

			t = renderer3D.getEntityTransform(controlPoints[1]);
			t.position = point2;
			renderer3D.setEntityTransform(controlPoints[1], t);

			t = renderer3D.getEntityTransform(controlPoints[2]);
			t.position = point3;
			renderer3D.setEntityTransform(controlPoints[2], t);

			t = renderer3D.getEntityTransform(controlPoints[3]);
			t.position = point4;
			renderer3D.setEntityTransform(controlPoints[3], t);

			t = renderer3D.getEntityTransform(sphereEntity);
			float beziePoint = anim;

			for (int i = 0; i < 100; i++)
			{
				auto rez = bezie(pg1, pg2, pg3, pg4, i / 100.f);
			
				auto closeness = (rez.x - anim);
			
				if (closeness < 0.01)
				{
					beziePoint = rez.y;
					graphPoint = rez;
				}
			}
			//beziePoint = bezie(pg1, pg2, pg3, pg4, anim).y;
			//graphPoint = bezie(pg1, pg2, pg3, pg4, anim);

			t.position = bezie(point1, point2, point3, point4, beziePoint);
			renderer3D.setEntityTransform(sphereEntity, t);

		}

		//clear 2D immage
		for (int i = 0; i < sizeof(graph); i += 4)
		{
			graph[i + 0] = 0.3 * 255;
			graph[i + 1] = 0.3 * 255;
			graph[i + 2] = 0.3 * 255;
			graph[i + 3] = 255;
		}

		auto drawPixel = [&](glm::vec2 point, glm::vec3 color)
		{
			if (point.x < 0 || point.x >= 1 || point.y < 0 || point.y >= 1) { return; }

			int index = (int(point.x * 128) + int(point.y * 128) * 128) * 4;

			graph[index + 0] = color[0] * 255;
			graph[index + 1] = color[1] * 255;
			graph[index + 2] = color[2] * 255;
		};

		auto drawCub = [&](glm::vec2 point, glm::vec3 color)
		{
			for (int i = -2; i < 2; i++)
				for (int j = -2; j < 2; j++)
				{
					drawPixel(point + glm::vec2(i, j) / 128.f, color);
				}
		};

		//render bezie points
		for (int i = 0; i < 300; i++)
		{
			auto rez = bezie(pg1, pg2, pg3, pg4, i / 300.f);
			//auto rez = glm::mix(pg1, pg4, i / 300.f);

			drawPixel(rez, {1,0,0});
		}

		drawCub(pg1, glm::vec3(0, 0, 1));
		drawCub(pg2, glm::vec3(1, 1, 0));
		drawCub(pg3, glm::vec3(0, 1, 1));
		drawCub(pg4, glm::vec3(1, 0, 1));
			
		drawCub(graphPoint, glm::vec3(0, 1, 0));


		t.bind();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, graph);


		ImGui::PushID(requestedInfo.requestedImguiIds);
		if (ImGui::Begin("General3DEditor"))
		{
			ImGui::DragFloat3("Point 1", glm::value_ptr(point1), 0.1);
			ImGui::DragFloat3("Point 2", glm::value_ptr(point2), 0.1);
			ImGui::DragFloat3("Point 3", glm::value_ptr(point3), 0.1);
			ImGui::DragFloat3("Point 4", glm::value_ptr(point4), 0.1);

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();

			ImGui::DragFloat2("Speed 1", glm::value_ptr(pg1), 0.01);
			ImGui::DragFloat2("Speed 2", glm::value_ptr(pg2), 0.01);
			ImGui::DragFloat2("Speed 3", glm::value_ptr(pg3), 0.01);
			ImGui::DragFloat2("Speed 4", glm::value_ptr(pg4), 0.01);

			pg1 = glm::clamp(pg1, {0,0,0}, {1,1,1});
			pg2 = glm::clamp(pg2, {0,0,0}, {1,1,1});
			pg3 = glm::clamp(pg3, {0,0,0}, {1,1,1});
			pg4 = glm::clamp(pg4, {0,0,0}, {1,1,1});

			ImGui::Image((ImTextureID)t.id, {512,512}, {0,1}, {1,0});

			ImGui::SliderFloat("Time", &anim, 0, 1);

			ImGui::Checkbox("Anim Running: ", &animRunning);

			if (ImGui::Button("Reset anim"))
			{
				animRunning = 1;
				anim = 0;
			}

		}
		ImGui::End();
		ImGui::PopID();

		//ImGui::SetAllocatorFunctions(userMalloc, userFree);

		//ImGui::Begin("window from gameplay");
		//ImGui::Spinner("spinner", 10, 2);
		//ImGui::ProgressBar(0.4);
		//ImGui::BufferingBar("buffering bar", 0.4, {100, 5});
		//ImGui::LoadingIndicatorCircle("circle", 20, 8, 8);
		//ImGui::End();
		
		//ImGui::ShowDemoWindow();


		renderer3D.render(input.deltaTime);
		renderer.flush();

		return true;
	}

	//optional
	void destruct(RequestedContainerInfo &requestedInfo)
	{

	}

};

//todo flag to clear screen from engine
//todo error popup
//todo error popup disable in release