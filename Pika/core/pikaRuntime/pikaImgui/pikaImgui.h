#pragma once
#include <pikaConfig.h>


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imguiThemes.h"

#include <GLFW/glfw3.h>

namespace pika
{

	struct ImguiAndGlfwContext
	{
		using glfwMakeContextCurrent_t = decltype(glfwMakeContextCurrent);

		glfwMakeContextCurrent_t *glfwMakeContextCurrentPtr = {};
		GLFWwindow *wind = {};
		ImGuiContext *ImGuiContext = {};
	};

	ImGuiContext *initImgui(ImguiAndGlfwContext imguiAndGlfwContext);
	void setContext(ImguiAndGlfwContext imguiAndGlfwContext);
	void imguiStartFrame(ImguiAndGlfwContext imguiAndGlfwContext);
	void imguiEndFrame(ImguiAndGlfwContext imguiAndGlfwContext);


};
