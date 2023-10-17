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

		if (sizeCalculationType == Transform::normalSize)
		{

		}else if(sizeCalculationType == Transform::useAspectRatioOnY)
		{
			size.y = size.x * aspectRation;
		}else if (sizeCalculationType == Transform::useAspectRatioOnX)
		{
			size.x = size.y * aspectRation;
		}
		else if (sizeCalculationType == Transform::useAspectRatioOnYKeepMinimum)
		{
			float newSize = size.x * aspectRation;
			if(newSize > size.y)
			{
				float minimizeFactor = newSize / size.y;
				size.y = newSize;
				size /= minimizeFactor;
			}
			else
			{
				size.y = newSize;
			}
		}
		else if (sizeCalculationType == Transform::useAspectRatioOnXKeepMinimum)
		{
			float newSize = size.y * aspectRation;
			if (newSize > size.x)
			{
				float minimizeFactor = newSize / size.x;
				size.x = newSize;
				size /= minimizeFactor;
			}
			else
			{
				size.x = newSize;
			}
		}
		else
		{
			//toto error
		}

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

			case topRight:
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

		return {pos, size};
	}

	void Background::render(::gl2d::Renderer2D &renderer, glm::vec4 pos)
	{
		renderer.renderRectangle(pos, color);
	}

	void Text::render(gl2d::Renderer2D &renderer, glm::vec4 box, gl2d::Font &f)
	{
		if (!text.empty())
		{
			glm::vec4 finalPos = transform.applyTransform(box);
			auto s = renderer.determineTextRescaleFit(text, f, box);

			finalPos.x += finalPos.z / 2.f;
			finalPos.y += finalPos.w / 2.f;

			renderer.renderText(finalPos, text.c_str(), f, color, s);

			renderer.renderRectangle({finalPos.x-2, finalPos.y-2, 4, 4}, Colors_Red);
		}
	}


	void renderSushiElement(gl2d::Renderer2D &renderer, glm::vec4 box, sushi::Background &background, Text &text,
		gl2d::Font &font)
	{
		//backgrouund
		background.render(renderer, box);

		text.render(renderer, box, font);
	}

};