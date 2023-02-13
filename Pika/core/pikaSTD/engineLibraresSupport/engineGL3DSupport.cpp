#include "engineGL3DSupport.h"



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
