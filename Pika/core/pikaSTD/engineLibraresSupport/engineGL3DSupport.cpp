#include "engineGL3DSupport.h"
#include <stringManipulation/stringManipulation.h>

#undef min
#undef max

void errorCallbackCustom(std::string err, void *userData)
{
	RequestedContainerInfo *data = (RequestedContainerInfo *)userData;

	data->consoleWrite((err + "\n").c_str());
}

std::string readEntireFileCustom(const char *fileName, bool &couldNotOpen, void *userData)
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
	buffer.resize(size + 1);

	if (!data->readEntireFile(fileName, &buffer.at(0), size))
	{
		couldNotOpen = true;
		return "";
	}

	return buffer;
}

std::vector<char> readEntireFileBinaryCustom(const char *fileName, bool &couldNotOpen, void *userData)
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

bool defaultFileExistsCustom(const char *fileName, void *userData)
{
	RequestedContainerInfo *data = (RequestedContainerInfo *)userData;
	size_t s = 0;
	return data->getFileSizeBinary(fileName, s);
}

void pika::gl3d::generalSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer)
{
	ImGui::PushID(imguiId);
	//ImGui::SliderFloat("Gama Corections", &gamaCorection, 1, 3);
	ImGui::SliderFloat("Exposure", &renderer.internal.lightShader.lightPassUniformBlockCpuData.exposure, 0.1, 10);

	auto normalMap = renderer.isNormalMappingEnabeled();
	ImGui::Checkbox("Normal map", &normalMap);
	renderer.enableNormalMapping(normalMap);

	static bool lightSubScater = renderer.isLightSubScatteringEnabeled();
	ImGui::Checkbox("Light sub scater", &lightSubScater);
	renderer.enableLightSubScattering(lightSubScater);

	ImGui::Checkbox("Adaptive resolution", &renderer.adaptiveResolution.useAdaptiveResolution);
	ImGui::Text("Adaptive rez ratio: %.1f", renderer.adaptiveResolution.rezRatio);
	//ImGui::Checkbox("Z pre pass", &renderer.zPrePass);
	ImGui::Checkbox("Frustum culling", &renderer.frustumCulling);
	
	ImGui::PopID();
}

void pika::gl3d::fxaaSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer)
{
	ImGui::PushID(imguiId);

	ImGui::Checkbox("FXAA", &renderer.antiAlias.usingFXAA);

	auto &fxaaData = renderer.getFxaaSettings();

	ImGui::DragFloat("edgeDarkTreshold", &fxaaData.edgeDarkTreshold, 0.001, 0, 1);
	ImGui::DragFloat("edgeMinTreshold", &fxaaData.edgeMinTreshold, 0.001, 0, 1);
	ImGui::DragFloat("quaityMultiplier", &fxaaData.quaityMultiplier, 0.001, 0, 1);
	ImGui::DragInt("ITERATIONS", &fxaaData.ITERATIONS, 1, 1, 32);
	ImGui::DragFloat("SUBPIXEL_QUALITY", &fxaaData.SUBPIXEL_QUALITY, 0.001, 0, 1);

	ImGui::PopID();
}

void pika::gl3d::ssaoSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer)
{
	ImGui::PushID(imguiId);

	ImGui::Checkbox("SSAO", &renderer.internal.lightShader.useSSAO);
	ImGui::SliderFloat("SSAO bias", &renderer.internal.ssao.ssaoShaderUniformBlockData.bias, 0, 0.5);
	ImGui::SliderFloat("SSAO radius", &renderer.internal.ssao.ssaoShaderUniformBlockData.radius, 0, 2);
	ImGui::SliderInt("SSAO sample count", &renderer.internal.ssao.ssaoShaderUniformBlockData.samplesTestSize, 0, 64);
	ImGui::SliderFloat("SSAO exponent", &renderer.internal.ssao.ssao_finalColor_exponent, 0, 16);

	ImGui::PopID();
}

void pika::gl3d::ssrSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer)
{
	ImGui::PushID(imguiId);

	ImGui::Checkbox("SSR", &renderer.internal.hasLastFrameTexture);
	auto d = renderer.getSSRdata();

	ImGui::SliderFloat("max ray delta", &d.maxRayDelta, 0.0001f, 2.f); //for clamping infinity
	ImGui::SliderFloat("max ray step", &d.maxRayStep, 0.01f, 5.f);
	ImGui::SliderInt("max steps", &d.maxSteps, 5, 150);
	ImGui::SliderFloat("min ray step", &d.minRayStep, 0.001f, 1.f);
	ImGui::SliderInt("binary search steps", &d.numBinarySearchSteps, 2, 20);

	renderer.setSSRdata(d);

	ImGui::PopID();
}

void pika::gl3d::bloomSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer)
{
	ImGui::PushID(imguiId);

	ImGui::Checkbox("Bloom", &renderer.bloom());
	ImGui::DragFloat("Bloom tresshold", &renderer.internal.lightShader.lightPassUniformBlockCpuData.bloomTresshold,
		0.01, 0, 1);
	ImGui::DragFloat("Bloom intensity", &renderer.postProcess.bloomIntensty, 0.01, 0, 10);
	ImGui::Checkbox("High quality down sample", &renderer.bloomHighQualityDownSample());
	ImGui::Checkbox("High quality up sample", &renderer.bloomHighQualityUpSample());

	ImGui::PopID();
}

void pika::gl3d::chromaticAberationSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer)
{
	ImGui::PushID(imguiId);

	ImGui::Checkbox("Chromatic aberation", &renderer.chromaticAberationEnabeled());

	ImGui::DragFloat("Chromatic aberation strength", &renderer.postProcess.chromaticAberationStrength,
		1, 0, 200);

	ImGui::DragFloat("Chromatic aberation defocus", &renderer.postProcess.unfocusDistance,
		0.01, 0, 100);

	ImGui::PopID();
}

void pika::gl3d::lightEditorSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer)
{
	ImGui::PushID(imguiId);

	auto &pointLights = renderer.internal.pointLightIndexes;

	static int pointLightSelector = -1;
	ImGui::Text("Point lightd Count %d", pointLights.size());
	ImGui::InputInt("Current Point light:", &pointLightSelector);
	int n = ImGui::Button("New Light"); ImGui::SameLine();
	int remove = ImGui::Button("Remove Light");

	int lightSize = renderer.getPointLightShadowSize();
	ImGui::DragInt("Point light shadow texture size", &lightSize);
	renderer.setPointLightShadowSize(lightSize);

	if (pointLightSelector < -1)
	{
		pointLightSelector = -1;
	}

	if (n || (pointLightSelector >= (int)pointLights.size()))
	{
		//pointLights.push_back(
		renderer.createPointLight({0,0,0});
	}

	pointLightSelector = std::min(pointLightSelector, (int)pointLights.size() - 1);

	if (remove)
	{
		if (pointLightSelector >= 0)
		{
			::gl3d::PointLight light;
			light.id_ = pointLights[pointLightSelector];
			renderer.detletePointLight(light);
			pointLightSelector = std::min(pointLightSelector, (int)pointLights.size() - 1);
		}

	}

	ImGui::NewLine();

	if (pointLightSelector >= 0)
	{
		ImGui::PushID(12);

		::gl3d::PointLight light;
		light.id_ = pointLights[pointLightSelector];

		glm::vec3 color = renderer.getPointLightColor(light);
		ImGui::ColorEdit3("Color", &color[0]);
		renderer.setPointLightColor(light, color);

		glm::vec3 position = renderer.getPointLightPosition(light);
		ImGui::DragFloat3("Position", &position[0], 0.1);
		renderer.setPointLightPosition(light, position);

		float distance = renderer.getPointLightDistance(light);
		ImGui::DragFloat("Distance##point", &distance, 0.05, 0);
		renderer.setPointLightDistance(light, distance);

		float attenuation = renderer.getPointLightAttenuation(light);
		ImGui::DragFloat("Attenuation##point", &attenuation, 0.05, 0);
		renderer.setPointLightAttenuation(light, attenuation);

		float hardness = renderer.getPointLightHardness(light);
		ImGui::DragFloat("Hardness##point", &hardness, 0.05, 0.001);
		renderer.setPointLightHardness(light, hardness);

		bool shadows = renderer.getPointLightShadows(light);
		ImGui::Checkbox("Cast shadows##point", &shadows);
		renderer.setPointLightShadows(light, shadows);

		ImGui::PopID();
	}

	{
		ImGui::NewLine();

		auto &directionalLights = renderer.internal.directionalLightIndexes;

		static int directionalLightSelector = -1;
		ImGui::Text("Directional lightd Count %d", directionalLights.size());
		ImGui::InputInt("Current directional light:", &directionalLightSelector);
		int n = ImGui::Button("New Directional Light"); ImGui::SameLine();
		int remove = ImGui::Button("Remove Directional Light");

		int lightSize = renderer.getDirectionalLightShadowSize();
		ImGui::DragInt("Directional light shadow texture size", &lightSize);
		renderer.setDirectionalLightShadowSize(lightSize);

		if (directionalLightSelector < -1)
		{
			directionalLightSelector = -1;
		}

		if (n || directionalLightSelector >= (int)directionalLights.size())
		{
			renderer.createDirectionalLight(glm::vec3(0.f));
		}

		directionalLightSelector
			= std::min(directionalLightSelector, (int)directionalLights.size() - 1);

		if (remove)
		{
			if (directionalLightSelector >= 0)
			{
				::gl3d::DirectionalLight light;
				light.id_ = directionalLights[directionalLightSelector];

				renderer.deleteDirectionalLight(light);
				directionalLightSelector = std::min(directionalLightSelector, (int)directionalLights.size() - 1);
			}

		}

		ImGui::NewLine();

		if (directionalLightSelector >= 0)
		{
			::gl3d::DirectionalLight light;
			light.id_ = directionalLights[directionalLightSelector];

			ImGui::PushID(13);

			glm::vec3 color = renderer.getDirectionalLightColor(light);
			ImGui::ColorEdit3("Color##dir", &color[0]);
			renderer.setDirectionalLightColor(light, color);

			glm::vec3 direction = renderer.getDirectionalLightDirection(light);
			ImGui::DragFloat3("Direction##dir", &direction[0], 0.01);
			renderer.setDirectionalLightDirection(light, direction);

			float hardness = renderer.getDirectionalLightHardness(light);
			ImGui::SliderFloat("Hardness##dir", &hardness, 0.1, 10);
			renderer.setDirectionalLightHardness(light, hardness);

			bool castShadows = renderer.getDirectionalLightShadows(light);
			ImGui::Checkbox("Cast shadows##dir", &castShadows);
			renderer.setDirectionalLightShadows(light, castShadows);


			//ImGui::SliderFloat3("frustumSplit",
			//	&renderer.directionalShadows.frustumSplits[0], 0, 1);

			ImGui::PopID();
		}
	}

	{
		auto &spotLights = renderer.internal.spotLightIndexes;

		ImGui::NewLine();

		static int spotLightSelector = -1;
		ImGui::Text("Spot lightd Count %d", spotLights.size());
		ImGui::InputInt("Current spot light:", &spotLightSelector);
		int n = ImGui::Button("New Spot Light"); ImGui::SameLine();
		int remove = ImGui::Button("Remove Spot Light");

		int lightSize = renderer.getSpotLightShadowSize();
		ImGui::DragInt("Spot light shadow texture size", &lightSize);
		renderer.setSpotLightShadowSize(lightSize);

		if (spotLightSelector < -1)
		{
			spotLightSelector = -1;
		}

		if (n || spotLightSelector >= (int)spotLights.size())
		{

			renderer.createSpotLight({0,0,0}, glm::radians(90.f),
				{0,-1,0});
		}

		spotLightSelector
			= std::min(spotLightSelector, (int)spotLights.size() - 1);

		if (remove)
		{
			if (spotLightSelector >= 0)
			{
				::gl3d::SpotLight light;
				light.id_ = spotLights[spotLightSelector];

				renderer.deleteSpotLight(light);

				spotLightSelector = std::min(spotLightSelector,
					(int)renderer.internal.spotLights.size() - 1);
			}

		}

		ImGui::NewLine();

		if (spotLightSelector >= 0)
		{
			ImGui::PushID(14);

			::gl3d::SpotLight light;
			light.id_ = spotLights[spotLightSelector];

			glm::vec3 color = renderer.getSpotLightColor(light);
			ImGui::ColorEdit3("Color##spot", &color[0]);
			renderer.setSpotLightColor(light, color);

			glm::vec3 position = renderer.getSpotLightPosition(light);
			ImGui::DragFloat3("Position##spot", &position[0], 0.1);
			renderer.setSpotLightPosition(light, position);

			glm::vec3 direction = renderer.getSpotLightDirection(light);
			ImGui::DragFloat3("Direction##spot", &direction[0], 0.05);
			renderer.setSpotLightDirection(light, direction);

			float distance = renderer.getSpotLightDistance(light);
			ImGui::DragFloat("Distance##spot", &distance, 0.05, 0);
			renderer.setSpotLightDistance(light, distance);

			float attenuation = renderer.getSpotLightAttenuation(light);
			ImGui::DragFloat("Attenuation##spot", &attenuation, 0.05, 0);
			renderer.setSpotLightAttenuation(light, attenuation);

			float hardness = renderer.getSpotLightHardness(light);
			ImGui::DragFloat("Hardness##spot", &hardness, 0.05, 0, 20);
			renderer.setSpotLightHardness(light, hardness);


			float angle = renderer.getSpotLightFov(light);
			ImGui::SliderAngle("fov", &angle, 0, 180);
			renderer.setSpotLightFov(light, angle);

			bool castShadows = renderer.getSpotLightShadows(light);
			ImGui::Checkbox("Cast shadows##spot", &castShadows);
			renderer.setSpotLightShadows(light, castShadows);


			ImGui::PopID();
		}
	}



	ImGui::PopID();
}


void pika::gl3d::fpsInput(::gl3d::Renderer3D &renderer, pika::Input &input, float speed, glm::dvec2 &lastMousePos,
	RequestedContainerInfo &requestedInfo, glm::ivec2 windowSize)
{
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

	if (input.buttons[pika::Button::Q].held())
	{
		dir.y -= speed * input.deltaTime;
	}
	if (input.buttons[pika::Button::E].held())
	{
		dir.y += speed * input.deltaTime;
	}

	renderer.camera.moveFPS(dir);

	{
		if (input.hasFocus && input.rMouse.held())
		{
			glm::dvec2 currentMousePos = {input.mouseX, input.mouseY};

			float speed = 0.8f;

			glm::vec2 delta = lastMousePos - currentMousePos;
			delta *= speed * input.deltaTime;

			renderer.camera.rotateCamera(delta);

			//glm::ivec2 windowMid = windowSize / 2;
			//requestedInfo.setMousePositionRelevantToWindow(windowMid.x, windowMid.y);
			lastMousePos = currentMousePos;
		}
		else
		{
			lastMousePos = {input.mouseX, input.mouseY};
		}

	}
}


void pika::gl3d::General3DEditor::loadFromFile(::gl3d::Renderer3D &renderer, std::string file, RequestedContainerInfo &info)
{
	std::string data;

	if (info.readEntireFile(file.c_str(), data))
	{
		if (data.size() > 0)
		{
			renderer.loadSettingsFromJson(data.c_str(), 1, 0, 0);
		}
		pika::strlcpy(currentFile, file, sizeof(currentFile));
	}
}

void pika::gl3d::General3DEditor::update(int imguiId, ::gl3d::Renderer3D &renderer,
	pika::Input &input, float moveSpeed, RequestedContainerInfo &info, glm::ivec2 windowSize)
{
	ImGui::PushID(imguiId);
	if (ImGui::Begin("General3DEditor"))
	{
		if (ImGui::CollapsingHeader("Basic settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
		{
			pika::gl3d::generalSettingsWindow(imguiId, renderer);
		}

		if (ImGui::CollapsingHeader("fxaa settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
		{
			pika::gl3d::fxaaSettingsWindow(imguiId, renderer);
		}

		if (ImGui::CollapsingHeader("ssao settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
		{
			pika::gl3d::ssaoSettingsWindow(imguiId, renderer);
		}

		if (ImGui::CollapsingHeader("ssr settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
		{
			pika::gl3d::ssrSettingsWindow(imguiId, renderer);
		}

		if (ImGui::CollapsingHeader("bloom settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
		{
			pika::gl3d::bloomSettingsWindow(imguiId, renderer);
		}

		if (ImGui::CollapsingHeader("chromatic aberation settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
		{
			pika::gl3d::chromaticAberationSettingsWindow(imguiId, renderer);
		}

		if (ImGui::CollapsingHeader("lights settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
		{
			pika::gl3d::lightEditorSettingsWindow(imguiId, renderer);
		}

		if (ImGui::CollapsingHeader("Sky box", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding))
		{

			if (ImGui::Button("delete skybox"))
			{
				renderer.skyBox.clearTextures();
			}

			if (skyBoxFileSelector.run(imguiId))
			{
				auto ext = std::filesystem::path(skyBoxFileSelector.file).extension();
				if (ext == ".hdr") //todo pika to lower on strings
				{
					//todo api to log errors
					renderer.skyBox.clearTextures();
					renderer.skyBox = renderer.loadHDRSkyBox(skyBoxFileSelector.file);
				}
				else if (ext == ".png")
				{
					renderer.skyBox.clearTextures();
					renderer.skyBox = renderer.loadSkyBox(skyBoxFileSelector.file);
				}
			}


			ImGui::ColorEdit3("Global Ambient color", &renderer.skyBox.color[0]);

			ImGui::Separator();

			if (ImGui::Button("atmospheric scattering"))
			{
				renderer.skyBox.clearTextures();
				renderer.skyBox = renderer.atmosfericScattering(atmosphericScattering);
			}

			ImGui::DragFloat3("sun pos", &atmosphericScattering.sun[0], 0.1, -1, 1);
			ImGui::ColorEdit3("sky color", &atmosphericScattering.color1[0]);
			ImGui::ColorEdit3("sun color", &atmosphericScattering.color2[0]);
			ImGui::ColorEdit3("ground", &atmosphericScattering.ground[0]);
			ImGui::Checkbox("use ground", &atmosphericScattering.useGroundColor);
			ImGui::DragFloat("g", &atmosphericScattering.g, 0.1, 0, 50);

			float s = glm::length(atmosphericScattering.sun);
			if (s == 0)
			{
				atmosphericScattering.sun = {0,1,0};
			}
			else
			{
				atmosphericScattering.sun /= s;
			}

		}


		

		ImGui::InputText("current file", currentFile, sizeof(currentFile));
		if (ImGui::Button("save"))
		{
			saveToFile(renderer, info);
		}
		if (ImGui::Button("load"))
		{
			loadFromFile(renderer, currentFile, info);
		}


	}
	ImGui::End();
	ImGui::PopID();

	pika::gl3d::fpsInput(renderer, input, 4, lastMousePos, info, windowSize);


}

void pika::gl3d::General3DEditor::saveToFile(::gl3d::Renderer3D &renderer, RequestedContainerInfo &info)
{
	auto rez = renderer.saveSettingsToJson(true);

	info.writeEntireFile(currentFile, rez);
};
