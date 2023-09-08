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
	};

	void SushyBinaryFormat::addPieceInternal(sushi::Transform &transform)
	{
		addBinaryDataInternal(&transform, sizeof(transform));
	}

	void SushyBinaryFormat::addPieceInternal(sushi::Background &background)
	{
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
		addPieceInternal(el.transform);
		addPieceInternal(el.background);
		addBinaryDataInternal(el.name, sizeof(el.name));
		addBinaryDataInternal(&el.id, sizeof(el.id));
		addBinaryDataInternal(&el.layoutType, sizeof(el.layoutType));

		addMarkerInternal(markerChildrenIdList);
		addUIntArrayPieceInternal(el.orderedElementsIds);
	}

	void SushyBinaryFormat::addMainParentInternal(sushi::SushiParent &el)
	{
		addMarkerInternal(markerMainParent);
		addParentPieceInternal(el);
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
	//void SushyContext::signalElementToCacheInternl(SushiElement el)
	//{
	//	if (!el.hasValue())return;
	//	cachedData.insert({el.getName(), el});
	//}
	//
	//void SushyContext::signalElementToCacheToRemoveInternal(SushiElement el)
	//{
	//	if (!el.hasValue())return;
	//
	//	auto range = cachedData.equal_range(el.getName());
	//	for (auto it = range.first; it != range.second; ++it)
	//	{
	//		if (it->second.ptr == el.ptr)
	//		{
	//			cachedData.erase(it);
	//			break;
	//		}
	//	}
	//}

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

		//todo cache
		//signalElementToCacheToRemoveInternal(el);

		std::strncpy(el->name, newName, sizeof(el->name) - 1);

		//todo cache
		//signalElementToCacheInternl(el);
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
		auto parentsSize = parents.size();
		auto orderedElementsSize = orderedElementsIds.size();

		//todo signal error here
		assert(parentsSize == orderedElementsSize);

		std::vector<sushi::SushiParent*> toDraw;
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

		switch (layoutType)
		{
		case layoutFree:
		{
			for (int i = 0; i < toDrawSize; i++)
			{
				toDraw[i]->update(renderer, input, drawRegion);
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

				toDraw[i]->update(renderer, input, newPos);
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

				toDraw[i]->update(renderer, input, newPos);
			}
		}
		break;

		default:
		assert(0);
		break;
		}


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
		rez.traverseAddInternalMainParent(root);
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

		auto getNextTransformPiece = [&](Transform *buff) -> bool
		{
			return readBinaryData(buff, sizeof(Transform));
		};

		auto getNextBackgroundPiece = [&](Background *buff) -> bool
		{
			return readBinaryData(buff, sizeof(Background));
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
			if (!getNextTransformPiece(&parent.transform)) { return 0; }
			if (!getNextBackgroundPiece(&parent.background)) { return 0; }

			if (!readBinaryData(parent.name, sizeof(parent.name))) { return 0; }
			if (!readBinaryData(&parent.id, sizeof(parent.id))) { return 0; }
			if (!readBinaryData(&parent.layoutType, sizeof(parent.layoutType))) { return 0; }

			if (getNextMarker() != markerChildrenIdList) { return 0; }
			if (!getNextUnsignedIntVector(parent.orderedElementsIds)) { return 0; }

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

	bool SushyContext::load(SushyBinaryFormat &data)
	{
		*this = {};

		LoadeData loaded;

		if (!loadAllData(data, loaded)) { return 0; }

		if (loaded.mainParent)
		{
			unsigned int oldId = root.id;
			root = *loaded.mainParent;
			root.id = oldId;
		}

		SushiParent *parentToAddTo = &root;
		
		//todo if no loaded.mainParent we will make the first found parent root

		if (!loaded.parents.empty())
		{
		#pragma region find first parent
			SushiParent *firstParent = 0;
			if (loaded.mainParent)
			{
				firstParent = &(*loaded.mainParent);
			}
			else
				{
					//determine the main parent
					std::unordered_set<unsigned int> allparentsids;
					for (auto &p : loaded.parents)
					{
						if (allparentsids.find(p.id) != allparentsids.end())
						{
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
							}
						}
					}
					else
					{
						return 0;
					}

				}
		#pragma endregion

			if (!firstParent) { return 0; }

			struct ParentPair
			{
				SushiParent *element = 0;
				SushiParent *parentToAddTo = 0;
			};

			std::vector<ParentPair> parentsToAdd;
			parentsToAdd.push_back({firstParent, parentToAddTo});

			while (parentsToAdd.size())
			{
				auto currentP = *parentsToAdd.begin();
				parentsToAdd.erase(parentsToAdd.begin());

				addParent(*currentP.parentToAddTo, currentP.element->name, currentP.element->transform,
					currentP.element->background);

				bool found = 0;
				for (auto &id : currentP.element->orderedElementsIds)
				{

					if (!found)
					{
						for (auto &e : loaded.parents)
						{
							if (e.id == id)
							{
								parentsToAdd.push_back({&e,currentP.element});
								found = true;
								break;
							}
						}
					}
					if (found) { break; }
				}

				if (!found) { return 0; }

			}

		}

		
		
		return 1;
	}

#pragma endregion


};

