#pragma once
#include <glm/glm.hpp>
#include <gl2d/gl2d.h>

namespace sushi
{

	bool pointInBox(glm::vec2 p, glm::vec4 box);

	struct OutData
	{
		glm::vec4 absTransform = {};

		void set(glm::vec4 absTransform)
		{
			this->absTransform = absTransform;

		}
	};

	struct Transform
	{
		//enum
		//{
		//	RelativeTransform = 0,
		//	AbsoluteTransform,
		//};
		//
		//enum
		//{
		//	DimensionsPercentage = 0,
		//	DimensionsPixelsAbsolute,
		//};
		//
		//union
		//{
		//	glm::vec4 dimensions = {0,0,1,1};
		//	struct
		//	{
		//		glm::vec2 pos;
		//		glm::vec2 size;
		//	};
		//};
		//
		//int placementType = RelativeTransform;
		//int dimensionsType = DimensionsPercentage;

		glm::vec2 sizePixels = {0,0};
		glm::vec2 sizePercentage = {1,1};

		glm::vec2 positionPixels = {};
		glm::vec2 positionPercentage = {};

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

	struct Background
	{
		Background() {};
		Background(glm::vec4 color):color(color) {};
		Background(glm::vec4 color, gl2d::Texture texture):color(color), texture(texture) {};

		glm::vec4 color = {1,1,1,1};
		gl2d::Texture texture = {};

		void render(gl2d::Renderer2D &renderer, glm::vec4 pos);
	};

};