#include <sushi/sushiPrimitives.h>

namespace sushi
{
	void Transform::absoluteTransformPixelSize(glm::vec4 dimensions)
	{
		this->dimensions = dimensions;
		placementType = AbsoluteTransform;
		dimensionsType = DimensionsPixelsAbsolute;
	}
	
	void Transform::relativeTransformPixelSize(glm::vec4 dimensions)
	{
		this->dimensions = dimensions;
		placementType = RelativeTransform;
		dimensionsType = DimensionsPixelsAbsolute;
	}

	glm::vec4 Transform::applyTransform(glm::vec4 parent)
	{
		const glm::vec2 parentPos = parent;
		const glm::vec2 parentSize = {parent.z, parent.w};

		glm::vec2 pos = {};
		glm::vec2 size = {};

		switch (dimensionsType)
		{
		case DimensionsPercentage:
		{
			size = glm::vec2(dimensions.z, dimensions.w) * parentSize;
		}
		break;

		case DimensionsPixelsAbsolute:
		{
			size = glm::vec2(dimensions.z, dimensions.w);
		}
		break;

		default:
		return {};
		//todo special error functions
		};

		//determine position;
		switch (placementType)
		{
		case RelativeTransform:
		{
			pos = parentPos + glm::vec2(dimensions);
		}
		break;

		case AbsoluteTransform:
		{
			pos = glm::vec2(dimensions);
		}
		break;

		default:
		return {};
		//todo special error functions
		};




		return {pos, size};
	}



};