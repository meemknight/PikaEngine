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

		//remove
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
	b2Body *currentBodySelected = 0;

	glm::vec2 draggedStart = {};

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
	#pragma region clear stuff
		glClear(GL_COLOR_BUFFER_BIT);
		renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
	#pragma endregion

	#pragma region input
		::pika::gl2d::cameraController(renderer.currentCamera, input, 200);
	#pragma endregion

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

		glm::vec2 mouseWorldpos(input.mouseX, input.mouseY);
		{
			auto viewRect = renderer.getViewRect();

			glm::vec2 mousePosNormalized = mouseWorldpos / glm::vec2(windowState.frameBufferW, windowState.frameBufferH);
			
			mouseWorldpos = glm::vec2(viewRect) + mousePosNormalized * glm::vec2(viewRect.z, viewRect.w);
		}

	#pragma region body render

		for (b2Body *b = world.GetBodyList(); b; b = b->GetNext())
		{

			glm::vec4 color = Colors_Red;
			float thickness = 1;

			if (currentBodySelected == b) 
			{
				color = Colors_Blue;
				thickness = 2;
			}

			auto centerOfMass = b->GetWorldCenter();
			auto angleDegrees = glm::degrees(b->GetAngle());
			auto shapePos = b->GetTransform();

			renderer.renderRectangle({centerOfMass.x - thickness,centerOfMass.y - thickness,2* thickness,2* thickness}, 
				color, {}, angleDegrees);

			for (b2Fixture *f = b->GetFixtureList(); f; f = f->GetNext()) 
			{
				auto shape = f->GetShape();

				if (shape->TestPoint(shapePos, {mouseWorldpos.x,mouseWorldpos.y}) && input.lMouse.held())
				{
					currentBodySelected = b;
				}

				//the shape doesn't know anything about position so we have to move it from 0 0 
				if (shape->GetType() == b2Shape::Type::e_polygon) 
				{
					b2PolygonShape *poligon = dynamic_cast<b2PolygonShape*>(shape);
				
					if (poligon)
					{
						for (int i = 0; i < poligon->m_count; i++)
						{
							int j = (i + 1) % poligon->m_count;

							glm::vec2 p1 = {poligon->m_vertices[i].x,poligon->m_vertices[i].y};
							glm::vec2 p2 = {poligon->m_vertices[j].x,poligon->m_vertices[j].y};

							p1 += glm::vec2{centerOfMass.x, centerOfMass.y};
							p2 += glm::vec2{centerOfMass.x, centerOfMass.y};

							p1 = gl2d::rotateAroundPoint(p1, {centerOfMass.x,-centerOfMass.y}, angleDegrees);
							p2 = gl2d::rotateAroundPoint(p2, {centerOfMass.x,-centerOfMass.y}, angleDegrees);

							renderer.renderLine(p1, p2, color, thickness*2);
						}

					}

				}
			
			}


		}

	#pragma endregion

	#pragma region drag

		{

			if (input.rMouse.pressed())
			{
				draggedStart = glm::vec2(input.mouseX, input.mouseY);
			}

			if (input.rMouse.released())
			{
				glm::vec2 dragEnd(input.mouseX, input.mouseY);

				glm::vec2 movement = dragEnd - draggedStart;

				if (currentBodySelected)
				{
					currentBodySelected->SetLinearVelocity({movement.x,movement.y});
				}
			}
			
		}


	#pragma endregion


		renderer.flush();

		
		return true;
	}

	//optional
	void destruct()
	{

	}

};

