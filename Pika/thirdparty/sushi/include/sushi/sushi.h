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

		Layout layout;

		Text text;

		std::vector<SushiParent> parents;

		std::vector<unsigned int> orderedElementsIds;

		void update(gl2d::Renderer2D &renderer,
			sushi::SushiInput &input, glm::vec4 parentTransform, 
			gl2d::Font &font);

		OutData outData;

		bool deleteByIdInternal(unsigned int id);

		void addParentInternal(
			const char *name,
			Transform &transform,
			Background &background,
			Layout &layout,
			unsigned int id,
			SushiParent **outNewElement);

		SushiParent *getParentByIdInternal(unsigned int id);
	};

	struct SushyBinaryFormat
	{
		std::vector<char> data;

		void addTransformInternal(sushi::Transform &transform);

		void addColorInternal(glm::vec4 &color);

		void addBackgroundInternal(sushi::Background &background);
		
		void addParentPieceInternal(sushi::SushiParent &el);

		void addMainParentInternal(sushi::SushiParent &el);

		void addTextInternal(std::string &el);

		void addParentInternal(sushi::SushiParent &el);

		void addMarkerInternal(int marker);
		
		void addBinaryDataInternal(void *d, size_t s);

		void addLayourInternal(sushi::Layout &layout);

		void addUIntArrayPieceInternal(std::vector<unsigned int> &arr);

		void addTextInternal(sushi::Text &text);

		void save(SushiParent &parent, bool isMainParent);

		void traverseAddInternal(SushiParent &parent);

		void traverseAddInternalMainParent(SushiParent &parent);
	};

	//this is a sushi context. Holds all the windows and manages stuff
	struct SushyContext
	{
		void createBasicSchene(int baseId = 1, const char *name = "Root");

		SushiParent root = SushiParent{"Root", 1, sushi::Background(glm::vec4{0,0,0,1}, 
			gl2d::Texture{})};
		unsigned int currentIdCounter = 2;

		//todo option to use another rect than camera
		//draw regions are like this: x, y, w, h
		void update(gl2d::Renderer2D &renderer, 
			sushi::SushiInput &input, gl2d::Font &font);

		unsigned int addParent(
			SushiParent &parent,
			const char *name,
			Transform &transform,
			Background &background,
			Layout &layout,
			SushiParent **outNewElement = nullptr
			);

		unsigned int addParent(
			unsigned int parent,
			const char *name,
			Transform &transform,
			Background &background,
			Layout &layout,
			SushiParent **outNewElement = nullptr
		);

		SushiParent *getParentByid(unsigned int id);

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

		SushyBinaryFormat saveFromParent(SushiParent *parent);

		bool load(SushyBinaryFormat &data, unsigned int fromId = 0);

		void regenerateCache();
	};



};