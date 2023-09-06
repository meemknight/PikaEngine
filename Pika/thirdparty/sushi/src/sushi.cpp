#include <sushi/sushi.h>

namespace sushi
{

	unsigned int SushyContext::addElement(
		SushiParent &parent,
		const char *name,
		Transform &transform,
		Background &background)
	{
		unsigned int id = currentIdCounter++;
		parent.addElement(name, transform, background, id);
		return id;
	}

	unsigned int SushyContext::addParent(
		SushiParent &parent,
		const char *name,
		Transform &transform,
		Background &background)
	{
		unsigned int id = currentIdCounter++;
		parent.addParent(name, transform, background, id);
		return id;
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
		auto subUiElSize = parents.size();
		auto ordererElementsSize = orderedElementsIds.size();

		std::vector<sushi::SushiElement> toDraw;
		toDraw.reserve(ordererElementsSize);

		for (int i = 0; i < ordererElementsSize; i++)
		{
			int id = orderedElementsIds[i];

			for (int i = 0; i < uiElSize; i++)
			{
				if(allUiElements[i].id == id)
				{
					toDraw.push_back(sushi::SushiElement(&allUiElements[i]));
					//allUiElements[i].update(renderer, input, drawRegion);
					goto end;
				}
			}

			for (int i = 0; i < subUiElSize; i++)
			{
				if (parents[i].id == id)
				{
					toDraw.push_back(sushi::SushiElement(&parents[i]));
					//parents[i].update(renderer, input, drawRegion);
					goto end;
				}
			}

			end:
			;
		}

		auto toDrawSize = toDraw.size();

		switch (layoutType)
		{
		case layoutFree:
		{
			for (int i = 0; i < toDrawSize; i++)
			{
				if (toDraw[i].getParent())
				{
					toDraw[i].getParent()->update(renderer, input, drawRegion);
				}
				else if (toDraw[i].getUiElement())
				{
					toDraw[i].getUiElement()->update(renderer, input, drawRegion);
				}
			}
		}
		break;

		case layourVertical:
		{
			for (int i = 0; i < toDrawSize; i++)
			{
				glm::vec4 newPos = drawRegion;
				newPos.w /= toDrawSize;
				newPos.y += newPos.w * i;

				if (toDraw[i].getParent())
				{
					toDraw[i].getParent()->update(renderer, input, newPos);
				}
				else if (toDraw[i].getUiElement())
				{
					toDraw[i].getUiElement()->update(renderer, input, newPos);
				}
			}
		}
		break;

		case layoutHorizontal:
		{
			for (int i = 0; i < toDrawSize; i++)
			{
				glm::vec4 newPos = drawRegion;
				newPos.z /= toDrawSize;
				newPos.x += newPos.z * i;

				if (toDraw[i].getParent())
				{
					toDraw[i].getParent()->update(renderer, input, newPos);
				}
				else if (toDraw[i].getUiElement())
				{
					toDraw[i].getUiElement()->update(renderer, input, newPos);
				}
			}
		}
		break;

		default:
		assert(0);
		break;
		}


	}

	void SushiUiElement::update(gl2d::Renderer2D &renderer, 
		sushi::SushiInput &input, glm::vec4 parentTransform)
	{
		glm::vec4 rectRez = transform.applyTransform(parentTransform);
		outData.set(rectRez);

		background.render(renderer, rectRez);
	}

	bool SushiParent::deleteById(unsigned int id)
	{
		for (int i = 0; i < orderedElementsIds.size(); i++)
		{
			if (orderedElementsIds[i] == id)
			{
				orderedElementsIds.erase(orderedElementsIds.begin() + i);
			}
		}

		for (int i = 0; i < allUiElements.size(); i++)
		{
			if (allUiElements[i].id == id)
			{
				allUiElements.erase(allUiElements.begin() + i);
				return 1;
			}
		}

		for (int i = 0; i < parents.size(); i++)
		{
			if (parents[i].id == id)
			{
				parents.erase(parents.begin() + i);
				return 1;
			}

			if (parents[i].deleteById(id))
			{
				return 1;
			};
		}

		return 0;
	}

	void SushiParent::addElement(
		const char *name,
		Transform &transform,
		Background &background,
		unsigned int id)
	{
		sushi::SushiUiElement element;
		element.id = id;
		element.background = background;
		element.transform = transform;
		std::strncpy(element.name, name, sizeof(element.name) - 1);

		allUiElements.push_back(element);
		orderedElementsIds.push_back(id);
	}

	void SushiParent::addParent(
		const char *name,
		Transform &transform,
		Background &background,
		unsigned int id)
	{
		sushi::SushiParent newParent;
		newParent.id = id;
		newParent.background = background;
		newParent.transform = transform;
		std::strncpy(newParent.name, name, sizeof(newParent.name) - 1);

		parents.push_back(newParent);
		orderedElementsIds.push_back(id);
	}

};

