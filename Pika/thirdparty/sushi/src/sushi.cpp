#include <sushi/sushi.h>
#include <cstring>

namespace sushi
{

#pragma region binary format

	enum: int
	{
		markerUiElement = 1,
		markerParent,
		markerChildrenIdList
	};

	void SushyBinaryFormat::addPieceInternal(sushi::Transform &transform)
	{
		addBinaryDataInternal(&transform, sizeof(transform));
	}

	void SushyBinaryFormat::addPieceInternal(sushi::Background &background)
	{
		addBinaryDataInternal(&background, sizeof(background));
	}

	void SushyBinaryFormat::addUiElementInternal(sushi::SushiUiElement &el)
	{
		addMarkerInternal(markerUiElement);
		addBinaryDataInternal(&el, sizeof(el));
	}
	
	void SushyBinaryFormat::addUIntArrayPieceInternal(std::vector<unsigned int> &arr)
	{
		size_t s = arr.size();
		addBinaryDataInternal(&s, sizeof(s));
		addBinaryDataInternal(arr.data(), s * sizeof(unsigned int));
	}

	void SushyBinaryFormat::addParentInternal(sushi::SushiParent &el)
	{
		addMarkerInternal(markerParent);
		addPieceInternal(el.transform);
		addPieceInternal(el.background);
		addBinaryDataInternal(el.name, sizeof(el.name));
		addBinaryDataInternal(&el.id, sizeof(el.id));
		addBinaryDataInternal(&el.layoutType, sizeof(el.layoutType));

		addMarkerInternal(markerChildrenIdList);
		addUIntArrayPieceInternal(el.orderedElementsIds);
	}

	void SushyBinaryFormat::addMarkerInternal(int marker)
	{
		addBinaryDataInternal(&marker, sizeof(marker));
	}

	void SushyBinaryFormat::addBinaryDataInternal(void *d, size_t s)
	{
		if (d && s)
		{
			auto beginPos = data.size();
			data.resize(beginPos + s);
			std::memcpy(&data[beginPos], d, s);
		}
	}

	void SushyBinaryFormat::traverseAddInternal(SushiParent &parent)
	{
		addParentInternal(parent);

		auto &ui = parent.allUiElements;
		for (auto &e : ui)
		{
			addUiElementInternal(e);
		}

		auto &parents = parent.parents;
		for (auto &e : parents)
		{
			traverseAddInternal(e);
		}
	}

	bool SushyBinaryFormat::save(SushiElement element)
	{
		data.clear();
		//todo reserve
		
		if (element.isUiElement())
		{
			addUiElementInternal(*element.getUiElement());
			return true;
		}
		else if (element.isParent())
		{
			traverseAddInternal(*element.getParent());
		}
		else
		{
			return false;
		}
	}

#pragma endregion


	void SushyContext::signalElementToCacheInternl(SushiElement el)
	{
		if (!el.hasValue())return;
		cachedData.insert({el.getName(), el});
	}

	void SushyContext::signalElementToCacheToRemoveInternal(SushiElement el)
	{
		if (!el.hasValue())return;

		auto range = cachedData.equal_range(el.getName());
		for (auto it = range.first; it != range.second; ++it)
		{
			if (it->second.ptr == el.ptr)
			{
				cachedData.erase(it);
				break;
			}
		}
	}

	SushiElement SushyContext::genUniqueElement(std::string name)
	{
		auto range = cachedData.equal_range(name);

		if (range.first == cachedData.end()) { return {}; }

		auto pos2 = range.first;
		pos2++;
		if (pos2 == range.second)
		{
			return range.first->second;
		}
		else
		{
			return {};
		}
	}

	std::pair<std::unordered_multimap<std::string, SushiElement>::iterator,
		std::unordered_multimap<std::string, SushiElement>::iterator> SushyContext::getElements(std::string name)
	{
		auto range = cachedData.equal_range(name);
		return range;
	}

	void SushyContext::rename(SushiElement el, char *newName)
	{
		if (!el.hasValue()) { return; }

		signalElementToCacheToRemoveInternal(el);

		if (el.isParent())
		{
			std::strncpy(el.getParent()->name, newName, sizeof(el.getParent()->name) - 1);
		}
		else if(el.isUiElement())
		{
			std::strncpy(el.getUiElement()->name, newName, sizeof(el.getUiElement()->name) - 1);
		}

		signalElementToCacheInternl(el);
		
	}

	unsigned int SushyContext::addElement(
		SushiParent &parent,
		const char *name,
		Transform &transform,
		Background &background)
	{



		unsigned int id = currentIdCounter++;
		parent.addElementInternal(name, transform, background, id);
		return id;
	}

	unsigned int SushyContext::addParent(
		SushiParent &parent,
		const char *name,
		Transform &transform,
		Background &background)
	{
		unsigned int id = currentIdCounter++;
		parent.addParentInternal(name, transform, background, id);
		return id;
	}

	bool SushyContext::deleteById(unsigned int id)
	{
		return root.deleteByIdInternal(id);
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

	bool SushiParent::deleteByIdInternal(unsigned int id)
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

			if (parents[i].deleteByIdInternal(id))
			{
				return 1;
			};
		}

		return 0;
	}

	void SushiParent::addElementInternal(
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

	void SushiParent::addParentInternal(
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

#pragma region save load

	SushyBinaryFormat SushyContext::save()
	{
		SushyBinaryFormat rez;

		//todo print errors or something?
		rez.save(&root);

		return rez;
	}

	bool SushyContext::load(SushyBinaryFormat &data)
	{
		*this = {};

		size_t cursorPos = 0;

		auto readBinaryData = [&](void *buffer, size_t size) -> bool
		{
			if (cursorPos + size > data.data.size())
			{
				return 0;
			}

			std::memcpy(buffer, &data.data[cursorPos], size);
			cursorPos += size;

			return true;
		};

		auto getNextMarker = [&]()
		{
			int marker = 0;
			readBinaryData(&marker, sizeof(marker));
			return marker;
		};

		auto getNextUiElementPiece = [&](SushiUiElement *buff) -> bool
		{
			return readBinaryData(buff, sizeof(SushiUiElement));
		};

		int firstM = getNextMarker();

		if (firstM == markerUiElement)
		{
			this->createBasicSchene();

			SushiUiElement el;

			if (!getNextUiElementPiece(&el)) { return 0; }

			this->addElement(root, el.name, el.transform, el.background);
		}
		else if (firstM == markerParent)
		{

		}
		else
		{
			return 0;
		}
	}

#pragma endregion


};

