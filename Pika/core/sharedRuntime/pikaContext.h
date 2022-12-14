#pragma once

#include <pikaAllocator/freeListAllocator.h>

struct GLFWwindow;
struct ImGuiContext;
//void glfwMakeContextCurrent(GLFWwindow *handle)

namespace pika
{
	struct PikaContext
	{
		//using glfwMakeContextCurrent_t = decltype(glfwMakeContextCurrent);
		using glfwMakeContextCurrent_t = void(GLFWwindow *);

		glfwMakeContextCurrent_t *glfwMakeContextCurrentPtr = {};
		GLFWwindow *wind = {};
		ImGuiContext *ImGuiContext = {};

		pika::memory::FreeListAllocator imguiAllocator;
	};
};