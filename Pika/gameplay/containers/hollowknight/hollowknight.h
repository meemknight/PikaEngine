#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <box2d/box2d.h>
#include <engineLibraresSupport/engineGL2DSupport.h>

struct Holloknight: public Container
{

	gl2d::Renderer2D renderer;
	gl2d::Texture sprites;
	gl2d::TextureAtlasPadding atlas;

	b2World world{{0, 10}};

	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);

		info.requestImguiFbo = true; 


		return info;
	}

	struct Block 
	{
		glm::vec4 dimensions = {};
		b2Body *dynamicBody = 0;
		float rotationRadians = 0;

		void create(b2World &world, glm::vec4 dimensions, bool dynamic)
		{
			this->dimensions = dimensions;
		
			b2BodyDef myBodyDef;
			if(dynamic)
				myBodyDef.type = b2_dynamicBody; //this will be a dynamic body

			dimensions.x += dimensions.z / 2.f;
			dimensions.y += dimensions.y / 2.f;

			myBodyDef.position.Set(dimensions.x, dimensions.y); //set the starting position
			myBodyDef.angle = 0; //set the starting angle

			dynamicBody = world.CreateBody(&myBodyDef);

			b2PolygonShape boxShape;
			boxShape.SetAsBox(dimensions.z/2, dimensions.w / 2);

			b2FixtureDef boxFixtureDef;
			boxFixtureDef.shape = &boxShape;
			boxFixtureDef.density = 1;
			dynamicBody->CreateFixture(&boxFixtureDef);
		}

		void updateMetrics(b2World &world) 
		{
			auto pos = dynamicBody->GetPosition();
			dimensions.x = pos.x;
			dimensions.y = pos.y;

			rotationRadians = dynamicBody->GetAngle();
		}

		void render(gl2d::Renderer2D &renderer, glm::vec4 color) 
		{
			renderer.renderRectangle({dimensions.x - dimensions.z / 2,dimensions.y - dimensions.w / 2,
				dimensions.z,dimensions.w}, color, {}, -glm::degrees(rotationRadians));
		}
	};

	Block floor;
	Block blocks[10];


	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{

		renderer.create(requestedInfo.requestedFBO.fbo);

		sprites = pika::gl2d::loadTextureWithPixelPadding(PIKA_RESOURCES_PATH "holloknight/sprites.png", requestedInfo, 80);
		atlas = gl2d::TextureAtlasPadding(12, 12, sprites.GetSize().x, sprites.GetSize().y);

		world.SetAllowSleeping(true);
		world.SetContinuousPhysics(true);

		floor.create(world, {-100, 300, 1000, 30}, false);
		
		for (int i = 0; i < 10; i++) 
		{
			blocks[i].create(world, {100 + sin(i)*50, -51*i, 50,50}, true);
		}


		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
		int32 velocityIterations = 6;
		int32 positionIterations = 2;
		world.Step(input.deltaTime, velocityIterations, positionIterations);

		floor.updateMetrics(world);

		for (int i = 0; i < 10; i++) 
		{
			blocks[i].updateMetrics(world);
		}


		floor.render(renderer, Colors_White);

		for (int i = 0; i < 10; i++)
		{
			blocks[i].render(renderer, Colors_Orange);
		}


		renderer.flush();

		
		return true;
	}

	//optional
	void destruct()
	{

	}

};

