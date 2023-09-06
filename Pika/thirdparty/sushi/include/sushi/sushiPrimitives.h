#pragma once
#include <glm/glm.hpp>
#include <gl2d/gl2d.h>

namespace sushi
{

	bool pointInBox(glm::vec2 p, glm::vec4 box);

	struct Transform
	{
		enum
		{
			RelativeTransform,
			AbsoluteTransform,
		};

		enum
		{
			DimensionsPercentage,
			DimensionsPixelsAbsolute,
		};

		union
		{
			glm::vec4 dimensions = {0,0,1,1};
			struct
			{
				glm::vec2 pos;
				glm::vec2 size;
			};
		};

		int placementType = RelativeTransform;
		int dimensionsType = DimensionsPercentage;

		void absoluteTransformPixelSize(glm::vec4 dimensions);
		void relativeTransformPixelSize(glm::vec4 dimensions);
		void relativeTransformDimensionsPercentage(glm::vec4 dimensions);

		glm::vec4 applyTransform(glm::vec4 parent);
	};

	Transform defaultTransform();

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