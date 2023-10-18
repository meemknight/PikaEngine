#include <sushi/sushi.h>
#include <cstring>
#include <optional>
#include <unordered_set>

namespace sushi
{

#pragma region binary format

	enum: int
	{
		markerParent = 1,
		markerChildrenIdList,
		markerMainParent,
		markerLayout,
		markerTransform,
		markerBackground,
		markerId,
		markerName,
		markerText,
		markerTextElement,
		markerColor,
	};

	void SushyBinaryFormat::addTransformInternal(sushi::Transform &transform)
	{
		addMarkerInternal(markerTransform);
		addBinaryDataInternal(&transform, sizeof(transform));
	}

	void SushyBinaryFormat::addColorInternal(glm::vec4 &color)
	{
		addMarkerInternal(markerColor);
		addBinaryDataInternal(&color, sizeof(color));
	}

	void SushyBinaryFormat::addBackgroundInternal(sushi::Background &background)
	{
		addMarkerInternal(markerBackground);
		addBinaryDataInternal(&background, sizeof(background));
	}

	void SushyBinaryFormat::addUIntArrayPieceInternal(std::vector<unsigned int> &arr)
	{
		size_t s = arr.size();
		addBinaryDataInternal(&s, sizeof(s));
		addBinaryDataInternal(arr.data(), s * sizeof(unsigned int));
	}

	void SushyBinaryFormat::addParentPieceInternal(sushi::SushiParent &el)
	{
		//todo don't add default data
		addTransformInternal(el.transform);
		addBackgroundInternal(el.background);
		addLayourInternal(el.layout);

		addMarkerInternal(markerName);
		addBinaryDataInternal(el.name, sizeof(el.name));

		addMarkerInternal(markerId);
		addBinaryDataInternal(&el.id, sizeof(el.id));

		addTextInternal(el.text);		

		addMarkerInternal(markerChildrenIdList);
		addUIntArrayPieceInternal(el.orderedElementsIds);
	}

	void  SushyBinaryFormat::addTextInternal(std::string &el)
	{
		addMarkerInternal(markerText);
		size_t s = el.size();
		addBinaryDataInternal((void *)&s, sizeof(size_t));
		addBinaryDataInternal(el.data(), s);
	}

	void  SushyBinaryFormat::addTextInternal(sushi::Text &el)
	{
		addMarkerInternal(markerTextElement);
		
		if (!el.text.empty()) 
		{
			addTextInternal(el.text);
		}
		
		addTransformInternal(el.transform);
	
		addColorInternal(el.color);
	}

	void SushyBinaryFormat::addMainParentInternal(sushi::SushiParent &el)
	{
		addMarkerInternal(markerMainParent);
		addParentPieceInternal(el);
	}

	void SushyBinaryFormat::addLayourInternal(sushi::Layout &layout)
	{
		addMarkerInternal(markerLayout);
		addBinaryDataInternal(&layout, sizeof(layout));
	}

	void SushyBinaryFormat::addParentInternal(sushi::SushiParent &el)
	{
		addMarkerInternal(markerParent);
		addParentPieceInternal(el);
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

		auto &parents = parent.parents;
		for (auto &e : parents)
		{
			traverseAddInternal(e);
		}
	}

	void SushyBinaryFormat::traverseAddInternalMainParent(SushiParent &parent)
	{
		addMainParentInternal(parent);

		auto &parents = parent.parents;
		for (auto &e : parents)
		{
			traverseAddInternal(e);
		}
	}


	void SushyBinaryFormat::save(SushiParent &parent, bool isMainParent)
	{
		data.clear();
		//todo reserve
		
		if (isMainParent)
		{
			traverseAddInternalMainParent(parent);
		}
		else
		{
			traverseAddInternal(parent);
		}

	}

#pragma endregion

	//todo
	void SushyContext::signalElementToCacheInternl(SushiParent *parent)
	{
		if (!parent)return;
		cachedData.insert({parent->name, parent});
	}

	void SushyContext::signalElementToCacheToRemoveInternal(SushiParent *parent)
	{
		if (!parent)return;
	
		auto range = cachedData.equal_range(parent->name);
		for (auto it = range.first; it != range.second; ++it)
		{
			if (it->second == parent)
			{
				cachedData.erase(it);
				break;
			}
		}
	}

	SushiParent *SushyContext::genUniqueParent(std::string name)
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
			return nullptr;
		}
	}

	std::pair<std::unordered_multimap<std::string, SushiParent *>::iterator,
		std::unordered_multimap<std::string, SushiParent *>::iterator> SushyContext::genParents(std::string name)
	{
		auto range = cachedData.equal_range(name);
		return range;
	}

	void SushyContext::rename(SushiParent *el, char *newName)
	{
		if (!el) { return; }

		signalElementToCacheToRemoveInternal(el);

		std::strncpy(el->name, newName, sizeof(el->name) - 1);

		signalElementToCacheInternl(el);
	}

	unsigned int SushyContext::addParent(
		SushiParent &parent,
		const char *name,
		Transform &transform,
		Background &background,
		Layout &layout,
		SushiParent **outNewElement)
	{

		for (auto &p : parent.parents)
		{
			signalElementToCacheToRemoveInternal(&p);
		}

		unsigned int id = currentIdCounter++;
		parent.addParentInternal(name, transform, background, layout, id, outNewElement);

		for (auto &p : parent.parents)
		{
			signalElementToCacheInternl(&p);
		}

		return id;
	}

	unsigned int SushyContext::addParent(
		unsigned int parent,
		const char *name,
		Transform &transform,
		Background &background,
		Layout &layout,
		SushiParent **outNewElement
	)
	{
		return addParent(*getParentByid(parent), name, transform, background, 
			layout, outNewElement);
	}

	SushiParent *SushyContext::getParentByid(unsigned int id)
	{
		return root.getParentByIdInternal(id);
	}

	SushiParent *SushiParent::getParentByIdInternal(unsigned int id)
	{
		if (id == this->id) { return this; }

		for (auto &p : parents)
		{
			auto rez = p.getParentByIdInternal(id);
			if (rez) { return rez; }
		}

		return nullptr;
	}

	bool SushyContext::deleteById(unsigned int id)
	{
		if (root.deleteByIdInternal(id))
		{
			regenerateCache();
			return 1;
		}

		return 0;
	}

	void SushyContext::createBasicSchene(int baseId, const char *name)
	{
		*this = {};
		root.id = baseId;
		currentIdCounter = baseId + 1;
		std::strncpy(root.name, name, sizeof(root.name)-1);
	
	}

	void sushi::SushyContext::update(gl2d::Renderer2D &renderer,
		sushi::SushiInput &input, gl2d::Font &font)
	{

		if (renderer.windowH == 0 || renderer.windowW == 0)
		{
			return;
		}

		glm::vec4 drawRegion = {0, 0, renderer.windowW, renderer.windowH};

		renderer.pushCamera();
		{
			root.update(renderer, input, drawRegion, font);
		}
		renderer.popCamera();
	}

	void sushi::SushiParent::update(gl2d::Renderer2D &renderer,
		sushi::SushiInput &input, glm::vec4 parentTransform, gl2d::Font &font)
	{

		glm::vec4 drawRegion = transform.applyTransform(parentTransform);
		{
			bool mouseIn = pointInBox({input.mouseX, input.mouseY}, drawRegion);

			auto lMouse = input.lMouse;
			auto rMouse = input.rMouse;

			if (!mouseIn)
			{
				lMouse.flags = 0;
				rMouse.flags = 0;
			}

			outData.set(drawRegion, mouseIn, lMouse, rMouse);
		}

		renderSushiElement(renderer, drawRegion, background, text, font);

	#pragma region update children
		auto parentsSize = parents.size();
		auto orderedElementsSize = orderedElementsIds.size();

		//todo signal error here
		assert(parentsSize == orderedElementsSize);

		std::vector<sushi::SushiParent *> toDraw;
		toDraw.reserve(orderedElementsSize);

		for (int i = 0; i < orderedElementsSize; i++)
		{
			int id = orderedElementsIds[i];

			for (int i = 0; i < parentsSize; i++)
			{
				if (parents[i].id == id)
				{
					toDraw.push_back(&parents[i]);
					//parents[i].update(renderer, input, drawRegion);
					break;
				}
			}
		}

		auto toDrawSize = toDraw.size();

		switch (layout.layoutType)
		{
		case Layout::layoutFree:
		{
			for (int i = 0; i < toDrawSize; i++)
			{
				toDraw[i]->update(renderer, input, drawRegion, font);
			}
		}
		break;

		case Layout::layourVertical:
		{
			for (int i = 0; i < toDrawSize; i++)
			{
				glm::vec4 newPos = drawRegion;
				newPos.w /= toDrawSize;
				newPos.y += newPos.w * i;

				toDraw[i]->update(renderer, input, newPos, font);
			}
		}
		break;

		case Layout::layoutHorizontal:
		{
			for (int i = 0; i < toDrawSize; i++)
			{
				glm::vec4 newPos = drawRegion;
				newPos.z /= toDrawSize;
				newPos.x += newPos.z * i;

				toDraw[i]->update(renderer, input, newPos, font);
			}
		}
		break;

		default:
		assert(0);
		break;
		}
	#pragma endregion

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

	void SushiParent::addParentInternal(
		const char *name,
		Transform &transform,
		Background &background,
		Layout &layout,
		unsigned int id,
		SushiParent **outNewElement)
	{
		sushi::SushiParent newParent;
		newParent.id = id;
		newParent.background = background;
		newParent.transform = transform;
		newParent.layout = layout;
		std::strncpy(newParent.name, name, sizeof(newParent.name) - 1);

		parents.push_back(newParent);
		orderedElementsIds.push_back(id);

		if (outNewElement)
		{
			*outNewElement = &parents.back();
		}
	}

	void traverseToAddToCache(SushyContext &c, SushiParent &p)
	{
		for (auto &child : p.parents)
		{
			c.cachedData.insert({child.name, &child});
		}

		for (auto &child : p.parents)
		{
			traverseToAddToCache(c, child);
		}
	}

	void SushyContext::regenerateCache()
	{
		cachedData.clear();
		traverseToAddToCache(*this, root);
	}

#pragma region save load

	SushyBinaryFormat SushyContext::save()
	{
		SushyBinaryFormat rez;

		//todo print errors or something?
		rez.traverseAddInternalMainParent(root);
		return rez;
	}

	SushyBinaryFormat SushyContext::saveFromParent(SushiParent *parent)
	{
		SushyBinaryFormat rez;

		//todo print errors or something?
		rez.traverseAddInternal(*parent);
		return rez;
	}

	struct LoadeData
	{
		std::vector<SushiParent> parents;
		std::optional<SushiParent> mainParent = std::nullopt;
	};

	bool loadAllData(SushyBinaryFormat &data, LoadeData &rezult)
	{
		rezult = {};
			
		size_t cursorPos = 0;

		auto peekBinaryData = [&](void *buffer, size_t size) -> bool
		{
			if (cursorPos + size > data.data.size())
			{
				return 0;
			}

			std::memcpy(buffer, &data.data[cursorPos], size);

			return true;
		};

		auto readBinaryData = [&](void *buffer, size_t size) -> bool
		{
			bool rez = peekBinaryData(buffer, size);

			if (rez)
			{
				cursorPos += size;
				return true;
			}
			else
			{
				return false;
			}
		};

		auto getNextMarker = [&]()
		{
			int marker = 0;
			readBinaryData(&marker, sizeof(marker));
			return marker;
		};

		auto peekNextMarker = [&]()
		{
			int marker = 0;
			peekBinaryData(&marker, sizeof(marker));
			return marker;
		};

		auto getNextTransformPiece = [&](Transform *buff) -> bool
		{
			return readBinaryData(buff, sizeof(Transform));
		};

		auto getNextLayoutPiece = [&](Layout *buff) -> bool
		{
			return readBinaryData(buff, sizeof(Layout));
		};

		auto getNextBackgroundPiece = [&](Background *buff) -> bool
		{
			return readBinaryData(buff, sizeof(Background));
		};

		auto getNextTextElementPiece = [&](sushi::Text *text) -> bool
		{


			while (true)
			{
				int m = peekNextMarker();

				switch (m)
				{
					case markerText:
					{
						int _ = getNextMarker();
						size_t textSize = 0;
						if (!readBinaryData(&textSize, sizeof(textSize))) { return 0; }
						text->text.resize(textSize);
						if (!readBinaryData(text->text.data(), textSize)) { return 0; }
						break;
					}

					case markerTransform:
					{
						int _ = getNextMarker();
						if (!getNextTransformPiece(&text->transform)) { return 0; }
						break;
					}

					case markerColor:
					{
						int _ = getNextMarker();
						if (!readBinaryData(&text->color, sizeof(text->color))) { return 0; }
						break;
					}

					default:
					return true;
					break;
				}
			}

			return true;
		};

		auto getNextUnsignedIntVector = [&](std::vector<unsigned int> &vec) -> bool
		{
			vec.clear();
			size_t elements = 0;
			if (!readBinaryData(&elements, sizeof(elements))) { return 0; }

			if (elements > 10'000'000) { return 0; } //probably corupted data

			vec.resize(elements);

			if (!readBinaryData(vec.data(), elements * sizeof(unsigned int))) { vec.clear(); return 0; }
			
			return 1;
		};

		auto getNextParentPiece = [&](SushiParent &parent) -> bool
		{

			while (true)
			{
				auto marker = peekNextMarker();

				if (marker == 0) { return true; }

				switch (marker)
				{
				case markerParent:
					return 1;
				break;

				case markerChildrenIdList:
				{
					int _ = getNextMarker();
					if (!getNextUnsignedIntVector(parent.orderedElementsIds)) { return 0; }
				}
				break;

				case markerMainParent:
					return 1;
				break;

				case markerLayout:
				{
					int _ = getNextMarker();
					if (!getNextLayoutPiece(&parent.layout)) { return 0; }

				}
				break;

				case markerTransform:
				{
					int _ = getNextMarker();
					if (!getNextTransformPiece(&parent.transform)) { return 0; }
				}
				break;

				case markerBackground:
				{
					int _ = getNextMarker();
					if (!getNextBackgroundPiece(&parent.background)) { return 0; }
				}
				break;

				case markerId:
				{
					int _ = getNextMarker();
					if (!readBinaryData(&parent.id, sizeof(parent.id))) { return 0; }
				}
				break;

				case markerName:
				{
					int _ = getNextMarker();
					if (!readBinaryData(parent.name, sizeof(parent.name))) { return 0; }
				}
				break;

				case markerTextElement:
				{
					int _ = getNextMarker();
					if (!getNextTextElementPiece(&parent.text)) { return 0; }
				}
				break;

				default:
				return 0;
				}


			}

			

			return true;
		};

		while (cursorPos < data.data.size())
		{
			int firstM = getNextMarker();

			if (firstM == markerParent)
			{
				SushiParent el;
				if (!getNextParentPiece(el)) { return 0; }
				rezult.parents.push_back(el);
			}
			else if (firstM == markerMainParent)
			{
				if (rezult.mainParent.has_value()) { return 0; }

				SushiParent el;
				if (!getNextParentPiece(el)) { return 0; }
				rezult.mainParent = el;
			}
			else
			{
				return 0;
			}
		}
		return 1;
	}

	bool SushyContext::load(SushyBinaryFormat &data, unsigned int fromId)
	{
		//*this = {};

		LoadeData loaded;

		if (!loadAllData(data, loaded)) { return 0; }

		if (fromId == 0)
		{
			if (loaded.mainParent)
			{
				*this = {};
				unsigned int oldId = root.id;
				root = *loaded.mainParent;
				root.id = oldId;
				root.orderedElementsIds = {};
			}
			else
			{
				*this = {};
			}
		};

		unsigned int addTo = root.id;
		if (fromId) { addTo = fromId; }

		if (!loaded.parents.empty())
		{
			struct ParentPair
			{
				SushiParent *element = 0;
				unsigned int parentToAddToId = 0;
			};

			std::vector<ParentPair> parentsToAdd;

		#pragma region find first parent
			if (loaded.mainParent)
			{
				for (auto &id : loaded.mainParent->orderedElementsIds)
				{
					bool found = 0;

					for (auto &e : loaded.parents)
					{
						if (e.id == id)
						{
							parentsToAdd.push_back({&e, addTo});
							found = true;
							break;
						}
					}

					//we need to find a match for all ids here or else the data is corrupted
					if (!found) { *this = {}; return 0; }
				}
			}
			else //not tested
			{
				SushiParent *firstParent = 0;

				//determine the main parent
				std::unordered_set<unsigned int> allparentsids;
				for (auto &p : loaded.parents)
				{
					if (allparentsids.find(p.id) != allparentsids.end())
					{
						*this = {};
						return 0;
					}
					allparentsids.insert(p.id);
				}

				for (auto &p : loaded.parents)
				{
					for (auto &id : p.orderedElementsIds)
					{
						auto f = allparentsids.find(id);
						if (f != allparentsids.end())
						{
							allparentsids.erase(f);
						}
					}
				}

				//we found the main parent id
				if (allparentsids.size() == 1)
				{
					unsigned int id = *allparentsids.begin();

					for (auto &p : loaded.parents)
					{
						if (p.id == id)
						{
							firstParent = &p;
							break;
						}
					}
				}
				else
				{
					*this = {};
					return 0;
				}

				parentsToAdd.push_back({firstParent, addTo});
			}
		#pragma endregion


			while (parentsToAdd.size())
			{
				auto currentP = *parentsToAdd.begin();
				parentsToAdd.erase(parentsToAdd.begin());

				auto newParentAddedId = addParent(currentP.parentToAddToId,
					currentP.element->name, currentP.element->transform,
					currentP.element->background, currentP.element->layout);

				for (auto &id : currentP.element->orderedElementsIds)
				{
					bool found = 0;

					for (auto &e : loaded.parents)
					{
						if (e.id == id)
						{
							parentsToAdd.push_back({&e, newParentAddedId});
							found = true;
							break;
						}
					}
					
					if (!found) { *this = {}; return 0; }
				}

			}

		}

		
		
		return 1;
	}

#pragma endregion


};

