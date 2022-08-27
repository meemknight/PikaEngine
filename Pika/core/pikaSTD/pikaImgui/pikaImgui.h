#pragma once
#include <pikaConfig.h>


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imguiThemes.h"

#include <GLFW/glfw3.h>

#include <pikaContext.h>

namespace pika
{


	ImGuiContext *initImgui(PikaContext pikaContext);
	void setContext(PikaContext pikaContext);
	void imguiStartFrame(PikaContext pikaContext);
	void imguiEndFrame(PikaContext pikaContext);


};
