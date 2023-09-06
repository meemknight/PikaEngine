#pragma once
#include <gl2d/gl2d.h>
#include <sushi/sushiPrimitives.h>
#include <sushi/sushiInput.h>

namespace sushi
{

	struct SushiUiElement;

	//this is a sushi context. Holds all the windows and manages stuff
	struct SushyContext
	{

		std::vector<SushiUiElement> allUiElements;

		//draw regions are like this: x, y, w, h
		void update(gl2d::Renderer2D &renderer, 
			sushi::SushiInput &input);

		glm::vec4 background = {0,0,0,1};

	};

	

	//this can also be a window or a button or whatever you want
	struct SushiUiElement
	{

		Transform transform;
		glm::vec4 color = {1,1,1,1};



	};



};