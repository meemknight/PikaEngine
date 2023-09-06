#pragma once
#include <glm/glm.hpp>

namespace sushi
{

	struct Transform
	{
		enum
		{
			NoTransformErr = 0,
			RelativeTransform,
			AbsoluteTransform,
		};

		enum
		{
			NoDimensionsErr = 0,
			DimensionsPercentage,
			DimensionsPixelsAbsolute,
		};

		union
		{
			glm::vec4 dimensions = {};
			struct
			{
				glm::vec2 pos;
				glm::vec2 size;
			};
		};

		int placementType = 0;
		int dimensionsType = 0;

		void absoluteTransformPixelSize(glm::vec4 dimensions);
		void relativeTransformPixelSize(glm::vec4 dimensions);

		glm::vec4 applyTransform(glm::vec4 parent);
	};


};