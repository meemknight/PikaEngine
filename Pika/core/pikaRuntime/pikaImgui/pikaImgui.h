#pragma once
#include <pikaConfig.h>


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imguiThemes.h"

#include <GLFW/glfw3.h>

namespace pika
{


	ImGuiContext *initImgui(GLFWwindow *wind);
	void imguiStartFrame();
	void imguiEndFrame(GLFWwindow *wind);


};
