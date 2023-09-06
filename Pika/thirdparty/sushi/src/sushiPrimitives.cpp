#include <sushi/sushiPrimitives.h>

namespace sushi
{
	bool pointInBox(glm::vec2 p, glm::vec4 box)
	{
		if 
			(
				(p.x >= box.x && p.x <= (box.x + box.z)) &&
				(p.y >= box.y && p.y <= (box.y + box.w))
			)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

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
	
	void Transform::relativeTransformDimensionsPercentage(glm::vec4 dimensions)
	{
		this->dimensions = dimensions;
		placementType = RelativeTransform;
		dimensionsType = DimensionsPercentage;
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

	void Background::render(::gl2d::Renderer2D &renderer, glm::vec4 pos)
	{
		renderer.renderRectangle(pos, color);
	}

	Transform defaultTransform()
	{
		Transform t;
		t.relativeTransformDimensionsPercentage({0,0,1,1});
		return Transform();
	}
};