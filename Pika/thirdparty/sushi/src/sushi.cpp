#include <sushi/sushi.h>

namespace sushi
{

	unsigned int SushyContext::addElement(
		SushiParent &parent,
		const char *name,
		Transform &transform,
		Background &background)
	{

		sushi::SushiUiElement element;
		element.id = currentIdCounter++;
		element.background = background;
		element.transform = transform;
		std::strncpy(element.name, name, sizeof(element.name) - 1);
		
		parent.allUiElements.push_back(element);

		return currentIdCounter - 1;
	}

	unsigned int SushyContext::addParent(
		SushiParent &parent,
		const char *name,
		Transform &transform,
		Background &background)
	{

		sushi::SushiParent newParent;
		newParent.id = currentIdCounter++;
		newParent.background = background;
		newParent.transform = transform;
		std::strncpy(newParent.name, name, sizeof(newParent.name) - 1);

		parent.subElements.push_back(newParent);

		return currentIdCounter - 1;
	}

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

