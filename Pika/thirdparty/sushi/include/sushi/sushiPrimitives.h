#pragma once
#include <glm/glm.hpp>
#include <gl2d/gl2d.h>
#include <sushi/sushiInput.h>

namespace sushi
{

	bool pointInBox(glm::vec2 p, glm::vec4 box);

	struct OutData
	{
		glm::vec4 absTransform = {};

		sushi::SushiButton lmouse;
		sushi::SushiButton rmouse;

		bool mouseIn = 0; 

		void set(glm::vec4 absTransform, bool mouseIn, sushi::SushiButton lmouse, sushi::SushiButton rmouse)
		{
			this->absTransform = absTransform;
			this->mouseIn = mouseIn;
			this->lmouse = lmouse;
			this->rmouse = rmouse;
		}
	};

	struct Transform
	{

		glm::vec2 sizePixels = {0,0};
		glm::vec2 sizePercentage = {1,1};

		glm::vec2 positionPixels = {};
		glm::vec2 positionPercentage = {};

		float aspectRation = 1.f;

		enum sizeCalculation
		{
			normalSize = 0,
			useAspectRatioOnX,
			useAspectRatioOnY,
			useAspectRatioOnXKeepMinimum,
			useAspectRatioOnYKeepMinimum,

		};

		int sizeCalculationType = 0;

		enum anchor
		{
			topLeft = 0,
			topMiddle,
			topRight,
			middleLeft,
			center,
			middleRight,
			bottomLeft,
			bottomMiddle,
			bottomRight,
			absolute,
		};

		int anchorPoint = topLeft;

		//void absoluteTransformPixelSize(glm::vec4 dimensions);
		//void relativeTransformPixelSize(glm::vec4 dimensions);
		//void relativeTransformDimensionsPercentage(glm::vec4 dimensions);

		glm::vec4 applyTransform(glm::vec4 parent);

		void changeSettings(Transform t, glm::vec4 parent)
		{
			//todo
			//this->placementType = t.placementType;
			//this->dimensionsType = t.dimensionsType;

			anchorPoint = t.anchorPoint;
		}
	};

	struct Layout
	{
		enum
		{
			layoutFree = 0,
			layoutHorizontal,
			layourVertical,
		};

		int layoutType = 0;
	};

	struct Background
	{
		Background() {};
		Background(glm::vec4 color):color(color) {};
		Background(glm::vec4 color, gl2d::Texture texture):color(color), texture(texture) {};

		glm::vec4 color = {1,1,1,1};
		gl2d::Texture texture = {};

		void render(gl2d::Renderer2D &renderer, glm::vec4 pos);
	};

	struct Text
	{
		std::string text;
		Transform transform;
		glm::vec4 color = {1,1,1,1};

		void render(gl2d::Renderer2D &renderer, glm::vec4 box, gl2d::Font &f);

	};

	void renderSushiElement(gl2d::Renderer2D &renderer, glm::vec4 box, sushi::Background &background, 
		Text &text, gl2d::Font &font);

};