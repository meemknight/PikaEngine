#pragma once

//todo remove includes here
#include "imgui.h"
#include <GLFW/glfw3.h>

namespace pika
{
	struct PikaContext
	{
		using glfwMakeContextCurrent_t = decltype(glfwMakeContextCurrent);

		glfwMakeContextCurrent_t *glfwMakeContextCurrentPtr = {};
		GLFWwindow *wind = {};
		ImGuiContext *ImGuiContext = {};
	};
};