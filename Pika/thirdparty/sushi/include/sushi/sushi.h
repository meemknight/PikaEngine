#pragma once
#include <gl2d/gl2d.h>
#include <sushi/sushiPrimitives.h>
#include <sushi/sushiInput.h>
#include <unordered_map>
#include <map>

namespace sushi
{

	//this can also be a window or a button or whatever you want
	struct SushiUiElement
	{
		SushiUiElement() {};
		SushiUiElement(const char *name, int id)
		{
			this->id = id;
			std::strncpy(this->name, name, sizeof(this->name) - 1);
		};

		char name[16] = {};
		unsigned int id = 0;
		Transform transform;
		Background background;

		void update(gl2d::Renderer2D &renderer,
			sushi::SushiInput &input, glm::vec4 parentTransform);

		OutData outData;

	};

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

		std::vector<SushiUiElement> allUiElements;

		std::vector<SushiParent> parents;

		std::vector<unsigned int> orderedElementsIds;

		void update(gl2d::Renderer2D &renderer,
			sushi::SushiInput &input, glm::vec4 parentTransform);

		OutData outData;

		bool deleteByIdInternal(unsigned int id);

		void addElementInternal(
			const char *name,
			Transform &transform,
			Background &background,
			unsigned int id);

		void addParentInternal(
			const char *name,
			Transform &transform,
			Background &background,
			unsigned int id);
	};

	//parent or ui
	struct SushiElement
	{
		SushiElement() {};
		SushiElement(void *ptr, int type):ptr(ptr), type(type) {};
		SushiElement(SushiUiElement *ptr):ptr(ptr), type(TypeUiElement) {};
		SushiElement(SushiParent *ptr):ptr(ptr), type(TypeParent) {};

		void *ptr = 0;
		int type = 0;

		enum Type
		{
			TypeUiElement = 1,
			TypeParent = 2,
		};

		SushiUiElement *getUiElement()
		{
			if (type == TypeUiElement)
			{
				return (SushiUiElement *)ptr;
			}
			else { return 0; }
		}

		SushiParent *getParent()
		{
			if (type == TypeParent)
			{
				return (SushiParent *)ptr;
			}
			else { return 0; }
		}

		bool isUiElement() { return type == TypeUiElement; }
		bool isParent() { return type == TypeParent; }
		bool hasValue() { return (type != 0) && (ptr != nullptr); }
		std::string getName()
		{
			if (hasValue())
			{
				if (isParent())
				{
					return getParent()->name;
				}else if (isUiElement())
				{
					return getUiElement()->name;
				}
			}
			else
			{
				return "";
			}
		}

	};

	struct SushyBinaryFormat
	{
		std::vector<char> data;

		void addUiElementInternal(sushi::SushiUiElement &el);

		void addPieceInternal(sushi::Transform &transform);

		void addPieceInternal(sushi::Background &background);
		
		void addParentPieceInternal(sushi::SushiParent &el);

		void addMainParentInternal(sushi::SushiParent &el);

		void addParentInternal(sushi::SushiParent &el);

		void addMarkerInternal(int marker);
		
		void addBinaryDataInternal(void *d, size_t s);

		void addUIntArrayPieceInternal(std::vector<unsigned int> &arr);

		bool save(SushiElement element);

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

		unsigned int addElement(
			SushiParent &parent,
			const char *name,
			Transform &transform,
			Background &background);

		unsigned int addParent(
			SushiParent &parent,
			const char *name,
			Transform &transform,
			Background &background);

		bool deleteById(unsigned int id);

		std::unordered_multimap<std::string, SushiElement> cachedData;

		void signalElementToCacheInternl(SushiElement el);

		void signalElementToCacheToRemoveInternal(SushiElement el);

		//can't search for root
		SushiElement genUniqueElement(std::string name);

		//can't search for root
		std::pair<std::unordered_multimap<std::string, SushiElement>::iterator,
			std::unordered_multimap<std::string, SushiElement>::iterator> getElements(std::string name);

		void rename(SushiElement el, char *newName);

		SushyBinaryFormat save();

		bool load(SushyBinaryFormat &data);

		
	};





};