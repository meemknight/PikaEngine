#include <sushi/sushi.h>

namespace sushi
{

	void SushyContext::createBasicSchene(int baseId, const char *name)
	{
		*this = {};
		root.id = baseId;
		currentIdCounter = baseId + 1;
		std::strncpy(root.name, name, sizeof(root.name)-1);

	
	}

	void sushi::SushyContext::update(gl2d::Renderer2D &renderer,
		sushi::SushiInput &input)
	{

		if (renderer.windowH == 0 || renderer.windowW == 0)
		{
			return;
		}

		glm::vec4 drawRegion = {0, 0, renderer.windowW, renderer.windowH};

		renderer.pushCamera();
		{
			root.update(renderer, input, drawRegion);

		}
		renderer.popCamera();
	}

	void sushi::SushiParent::update(gl2d::Renderer2D &renderer,
		sushi::SushiInput &input, glm::vec4 parentTransform)
	{

		glm::vec4 drawRegion = transform.applyTransform(parentTransform);
		outData.set(drawRegion);


		//backgrouund
		background.render(renderer, drawRegion);

		auto uiElSize = allUiElements.size();
		for (int i = 0; i < uiElSize; i++)
		{
			allUiElements[i].update(renderer, input, drawRegion);
		}

		auto subUiElSize = subElements.size();
		for (int i = 0; i < subUiElSize; i++)
		{
			subElements[i].update(renderer, input, drawRegion);
		}

	}

	void SushiUiElement::update(gl2d::Renderer2D &renderer, 
		sushi::SushiInput &input, glm::vec4 parentTransform)
	{
		glm::vec4 rectRez = transform.applyTransform(parentTransform);
		outData.set(rectRez);

		background.render(renderer, rectRez);
	}

};

