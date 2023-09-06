#include <sushi/sushiPrimitives.h>
#include <cassert>

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

	//void Transform::absoluteTransformPixelSize(glm::vec4 dimensions)
	//{
	//	this->dimensions = dimensions;
	//	placementType = AbsoluteTransform;
	//	dimensionsType = DimensionsPixelsAbsolute;
	//}
	//
	//void Transform::relativeTransformPixelSize(glm::vec4 dimensions)
	//{
	//	this->dimensions = dimensions;
	//	placementType = RelativeTransform;
	//	dimensionsType = DimensionsPixelsAbsolute;
	//}
	//
	//void Transform::relativeTransformDimensionsPercentage(glm::vec4 dimensions)
	//{
	//	this->dimensions = dimensions;
	//	placementType = RelativeTransform;
	//	dimensionsType = DimensionsPercentage;
	//}

	glm::vec4 Transform::applyTransform(glm::vec4 parent)
	{
		const glm::vec2 parentPos = parent;
		const glm::vec2 parentSize = {parent.z, parent.w};

		glm::vec2 pos = {};
		glm::vec2 size = parentSize * sizePercentage + sizePixels;

		auto leftAlign = [&]()
		{
			pos.x = parentPos.x;
		};

		auto topAlign = [&]()
		{
			pos.y = parentPos.y;
		};

		auto middleXAlign = [&]()
		{
			pos.x = parentPos.x + (parentSize.x / 2.f) - (size.x / 2.f);
		};

		auto middleyAlign = [&]()
		{
			pos.y = parentPos.y + (parentSize.y / 2.f) - (size.y / 2.f);
		};

		auto rightAlign = [&]()
		{
			pos.x = parentPos.x + parentSize.x - size.x;
		};

		auto bottomAlign = [&]()
		{
			pos.y = parentPos.y + parentSize.y - size.y;
		};

		switch (anchorPoint)
		{
			case topLeft:
			{
				leftAlign();
				topAlign();
			}
			break;

			case topMiddle:
			{
				middleXAlign();
				topAlign();
			}
			break;

			case toRight:
			{
				rightAlign();
				topAlign();
			}
			break;

			case middleLeft:
			{
				leftAlign();
				middleyAlign();
			}
			break;

			case center:
			{
				middleXAlign();
				middleyAlign();
			}
			break;

			case middleRight:
			{
				rightAlign();
				middleyAlign();
			}
			break;

			case bottomLeft:
			{
				leftAlign();
				bottomAlign();
			}
			break;

			case bottomMiddle:
			{
				middleXAlign();
				bottomAlign();
			}
			break;

			case bottomRight:
			{
				rightAlign();
				bottomAlign();
			}
			break;

			case absolute:
			{
				
				size = sizePixels;
			}
			break;
			default:
			//todo signal error in production
			assert(0); //invalid anchor point
			return {};
		}

		pos += positionPixels + parentSize * positionPercentage;
		//switch (dimensionsType)
		//{
		//case DimensionsPercentage:
		//{
		//	size = glm::vec2(dimensions.z, dimensions.w) * parentSize;
		//}
		//break;
		//
		//case DimensionsPixelsAbsolute:
		//{
		//	size = glm::vec2(dimensions.z, dimensions.w);
		//}
		//break;
		//
		//default:
		//return {};
		////todo special error functions
		//};
		//
		////determine position;
		//switch (placementType)
		//{
		//case RelativeTransform:
		//{
		//	pos = parentPos + glm::vec2(dimensions);
		//}
		//break;
		//
		//case AbsoluteTransform:
		//{
		//	pos = glm::vec2(dimensions);
		//}
		//break;
		//
		//default:
		//return {};
		////todo special error functions
		//};


		return {pos, size};
	}

	void Background::render(::gl2d::Renderer2D &renderer, glm::vec4 pos)
	{
		renderer.renderRectangle(pos, color);
	}

};