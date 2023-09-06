#pragma once
#include <gl2d/gl2d.h>
#include <sushi/sushiPrimitives.h>
#include <sushi/sushiInput.h>

namespace sushi
{

	struct SushiUiElement;

	struct OutData
	{
		glm::vec4 absTransform = {};

		void set(glm::vec4 absTransform)
		{
			this->absTransform = absTransform;

		}
	};

	struct SushiParent
	{
		SushiParent() {};
		SushiParent(const char *name, int id)
		{
			this->id = id;
			std::strncpy(this->name, name, sizeof(name) - 1);
		};
		SushiParent(const char *name, int id, Background b)
		{
			this->id = id;
			std::strncpy(this->name, name, sizeof(name) - 1);
			background = b;
		};

		Transform transform;
		Background background;
		char name[16] = {};
		unsigned int id = 0;

		std::vector<SushiUiElement> allUiElements;

		std::vector<SushiParent> subElements;

		void update(gl2d::Renderer2D &renderer,
			sushi::SushiInput &input, glm::vec4 parentTransform);
		
		OutData outData;


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

		void addElement(
			SushiParent &parent,
			const char *name,
			Transform &transform,
			Background &background);

		void addParent(
			SushiParent &parent,
			const char *name,
			Transform &transform,
			Background &background);

	};

	
	//this can also be a window or a button or whatever you want
	struct SushiUiElement
	{
		SushiUiElement() {};
		SushiUiElement(const char *name, int id) 
		{
			this->id = id;
			std::strncpy(this->name, name, sizeof(name)-1);
		};

		char name[16] = {};
		unsigned int id = 0;
		Transform transform;
		Background background;

		void update(gl2d::Renderer2D &renderer,
			sushi::SushiInput &input, glm::vec4 parentTransform);

		OutData outData;
	};



};