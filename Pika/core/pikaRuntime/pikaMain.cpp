#include <iostream>
#include <cstdio>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl2d/gl2d.h>
gl2d::Renderer2D renderer;

int main()
{

	if (!glfwInit())
	{
		std::cout << "problem initializing glfw";
	}

	//glfwSetErrorCallback(error_callback); todo

	GLFWwindow *window = glfwCreateWindow(640, 480, "Pika", NULL, NULL);
	if (!window)
	{
		std::cout << "problem initializing window";
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL())
	{
		std::cout << "problem initializing glad";
	}

	gl2d::init();
	renderer.create();

	while (!glfwWindowShouldClose(window))
	{

		glClear(GL_COLOR_BUFFER_BIT);

		gl2d::enableNecessaryGLFeatures();
		renderer.updateWindowMetrics(640, 480);
		renderer.renderRectangle({10,10, 100, 100}, Colors_Magenta);
		renderer.flush();

		glfwPollEvents();
		glfwSwapBuffers(window);
	}



	return 0;
}