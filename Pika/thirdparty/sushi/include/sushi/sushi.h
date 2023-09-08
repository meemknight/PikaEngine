#pragma once
#include <gl2d/gl2d.h>
#include <sushi/sushiPrimitives.h>
#include <sushi/sushiInput.h>
#include <unordered_map>
#include <map>

namespace sushi
{

	//this can also be a window or a button or whatever you want
	struct SushiParent
	{
		SushiParent() {};
		SushiParent(const char *name, int id)
		{
			this->id = id;
			std::strncpy(this->name, name, sizeof(this->name) - 1);
		};
		SushiParent(const char *name, int id, Background b)
		{
			this->id = id;
			std::strncpy(this->name, name, sizeof(this->name) - 1);
			background = b;
		};

		Transform transform;
		Background background;
		char name[16] = {};
		unsigned int id = 0;

		enum
		{
			layoutFree = 0,
			layoutHorizontal,
			layourVertical,
		};

		int layoutType = 0;

		std::vector<SushiParent> parents;

		std::vector<unsigned int> orderedElementsIds;

		void update(gl2d::Renderer2D &renderer,
			sushi::SushiInput &input, glm::vec4 parentTransform);

		OutData outData;

		bool deleteByIdInternal(unsigned int id);

		void addParentInternal(
			const char *name,
			Transform &transform,
			Background &background,
			unsigned int id);
	};

	struct SushyBinaryFormat
	{
		std::vector<char> data;

		void addPieceInternal(sushi::Transform &transform);

		void addPieceInternal(sushi::Background &background);
		
		void addParentPieceInternal(sushi::SushiParent &el);

		void addMainParentInternal(sushi::SushiParent &el);

		void addParentInternal(sushi::SushiParent &el);

		void addMarkerInternal(int marker);
		
		void addBinaryDataInternal(void *d, size_t s);

		void addUIntArrayPieceInternal(std::vector<unsigned int> &arr);

		void save(SushiParent &parent, bool isMainParent);

		void traverseAddInternal(SushiParent &parent);

		void traverseAddInternalMainParent(SushiParent &parent);
	};

	//this is a sushi context. Holds all the windows and manages stuff
	struct SushyContext
	{
		void createBasicSchene(int baseId = 1, const char *name = "Main Parent");

		SushiParent root = SushiParent{"Main Parent", 1, sushi::Background(glm::vec4{0,0,0,1}, 
			gl2d::Texture{})};
		unsigned int currentIdCounter = 2;

		//draw regions are like this: x, y, w, h
		void update(gl2d::Renderer2D &renderer, 
			sushi::SushiInput &input);

		unsigned int addParent(
			SushiParent &parent,
			const char *name,
			Transform &transform,
			Background &background);

		bool deleteById(unsigned int id);

		//todo push backs can invalidate data so just recreate the entire cache
		std::unordered_multimap<std::string, SushiParent*> cachedData; 

		void signalElementToCacheInternl(SushiParent *parent);

		void signalElementToCacheToRemoveInternal(SushiParent *parent);

		//can't search for root
		SushiParent *genUniqueParent(std::string name);

		//can't search for root
		std::pair<std::unordered_multimap<std::string, SushiParent*>::iterator,
			std::unordered_multimap<std::string, SushiParent*>::iterator> genParents(std::string name);

		void rename(SushiParent *el, char *newName);

		SushyBinaryFormat save();

		bool load(SushyBinaryFormat &data);

		void regenerateCache();
	};





};