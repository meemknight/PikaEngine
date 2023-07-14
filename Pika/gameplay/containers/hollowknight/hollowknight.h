#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <box2d/box2d.h>
#include <engineLibraresSupport/engineGL2DSupport.h>
#include <pikaImgui/pikaImgui.h>

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
		info.pushAnImguiIdForMe = true;

		return info;
	}

	struct Block 
	{

		b2Body *dynamicBody = 0;
		glm::vec2 getPosCenter() 
		{
			auto pos = dynamicBody->GetPosition();
			return {pos.x,pos.y};
		};

		glm::vec2 getSize()
		{
			auto f = dynamicBody->GetFixtureList(); if (!f) return {};
			auto s = f->GetShape(); if (!s) return {};

			if (s->GetType() == b2Shape::Type::e_polygon)
			{
				b2PolygonShape *poligon = dynamic_cast<b2PolygonShape *>(s);

				if (poligon)
				{
					glm::vec2 min{999999,999999};
					glm::vec2 max{-999999,-999999};

					for (int i = 0; i < poligon->m_count; i++)
					{
						glm::vec2 p1 = {poligon->m_vertices[i].x,poligon->m_vertices[i].y};
						
						if (p1.x > max.x) { max.x = p1.x; }
						if (p1.y > max.y) { max.y = p1.y; }

						if (p1.x < min.x) { min.x = p1.x; }
						if (p1.y < min.y) { min.y = p1.y; }
					}

					glm::vec2 size = max - min;

					return size;
				}

			}

			return {};
		}

		glm::vec2 getPosTopLeft()
		{
			auto pos = dynamicBody->GetPosition();
			return glm::vec2(pos.x,pos.y) - getSize()/2.f;
		};

		float getRotation()
		{
			return dynamicBody->GetAngle();
		}

		void create(b2World &world, glm::vec4 dimensions, bool dynamic)
		{
			b2BodyDef myBodyDef;
			if (dynamic)
				myBodyDef.type = b2_dynamicBody; //this will be a dynamic body

			dimensions.x += dimensions.z / 2.f;
			dimensions.y += dimensions.y / 2.f;

			myBodyDef.position.Set(dimensions.x, dimensions.y); //set the starting position
			myBodyDef.angle = 0; //set the starting angle

			dynamicBody = world.CreateBody(&myBodyDef);

			b2PolygonShape boxShape;
			boxShape.SetAsBox(dimensions.z / 2, dimensions.w / 2);

			b2FixtureDef boxFixtureDef;
			boxFixtureDef.shape = &boxShape;
			boxFixtureDef.density = 1;
			dynamicBody->CreateFixture(&boxFixtureDef);
		}

		b2Fixture *addSensor(glm::vec4 dimensionsRelativeToCenter)
		{
			b2PolygonShape boxShape;

			b2Vec2 pos(dimensionsRelativeToCenter.x, dimensionsRelativeToCenter.y);

			b2Vec2 topLeft(-dimensionsRelativeToCenter.z / 2, dimensionsRelativeToCenter.w / 2);
			b2Vec2 bottomLeft(-dimensionsRelativeToCenter.z / 2, -dimensionsRelativeToCenter.w / 2);
			b2Vec2 bottomRight(dimensionsRelativeToCenter.z / 2, -dimensionsRelativeToCenter.w / 2);
			b2Vec2 topRight(dimensionsRelativeToCenter.z / 2, dimensionsRelativeToCenter.w / 2);

			b2Vec2 points[4] =
			{
				topLeft + pos,
				bottomLeft + pos,
				bottomRight + pos,
				topRight + pos
			};

			boxShape.Set(points, 4);

			b2FixtureDef boxFixtureDef;
			boxFixtureDef.shape = &boxShape;
			boxFixtureDef.density = 0;
			boxFixtureDef.isSensor = true;

			b2Fixture * f = dynamicBody->CreateFixture(&boxFixtureDef);
			return f;
		}

		void render(gl2d::Renderer2D &renderer, glm::vec4 color) 
		{
			glm::vec4 dimensions(getPosCenter(), getSize());
			renderer.renderRectangle({dimensions.x - dimensions.z / 2,dimensions.y - dimensions.w / 2,
				dimensions.z,dimensions.w}, color, {}, -glm::degrees(getRotation()));
		}
	};

	struct Entity
	{
		Block physicalBody;
		
		glm::vec4 spriteDimensions = {};

		glm::vec4 getRenderPos() 
		{
			glm::vec4 centerPos = glm::vec4(physicalBody.getPosCenter() + glm::vec2(spriteDimensions),
				glm::vec2(spriteDimensions.z, spriteDimensions.w));

			centerPos.x -= centerPos.z / 2.f;
			centerPos.y -= centerPos.w / 2.f;

			return centerPos;
		}
	};

	Block floor;
	Block blocks[10];
	b2Body *currentBodySelected = 0;

	Entity character;
	b2Fixture *characterJumpSensor;

	glm::vec2 draggedStart = {};

	bool followPlayer = true;

	struct InputMetrict
	{
		float speed = 20;
		float jump = 7;
		float stopSpeed = 7;
		float jumpTimer = 1.5;
		float initialJumpImpulse = 1;
	} inputMetrics = {};

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{

		renderer.create(requestedInfo.requestedFBO.fbo);
		renderer.currentCamera.zoom = 80.f;
		renderer.currentCamera.position.x = -440;
		renderer.currentCamera.position.y = -500;

		if (!requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "hollowknight/inputMetrics.bin",
			&inputMetrics, sizeof(inputMetrics)))
		{
			inputMetrics = {};
		}

		sprites = pika::gl2d::loadTextureWithPixelPadding(PIKA_RESOURCES_PATH "hollowknight/sprites.png", requestedInfo, 80);
		atlas = gl2d::TextureAtlasPadding(12, 12, sprites.GetSize().x, sprites.GetSize().y);

		world.SetAllowSleeping(true);
		world.SetContinuousPhysics(true);

		floor.create(world, {-50, 10, 100, 1}, false);
		
		for (int i = 0; i < 10; i++) 
		{
			blocks[i].create(world, {1+ sin(i)*5, -1.5*i, 1,1}, true);
		}

		character.physicalBody.create(world, {10, 2, 0.6f,1}, true);
		character.spriteDimensions = glm::vec4{0,0,1,1};
		character.physicalBody.dynamicBody->SetFixedRotation(true);
		characterJumpSensor = character.physicalBody.addSensor({0, 0.5, 0.55f, 0.1f});

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

		::pika::gl2d::cameraController(renderer.currentCamera, input, 60 * (!followPlayer), 4);

		int direction = 0;

		if (input.buttons[pika::Button::Left].held() 
			|| input.anyController.buttons[pika::Controller::Left].held() 
			|| input.anyController.LStick.left()
			) { direction--; }

		if (input.buttons[pika::Button::Right].held() 
			|| input.anyController.buttons[pika::Controller::Right].held()
			|| input.anyController.LStick.right()
			) { direction++; }

		{
			b2Vec2 vel = character.physicalBody.dynamicBody->GetLinearVelocity();
			float force = 0;


			if (direction < 0)
			{
				if (vel.x > -5) force = -inputMetrics.speed;
			}
			else if (direction == 0)
			{
				force = vel.x * -inputMetrics.stopSpeed;
			}
			else
			{
				if (vel.x < 5) force = inputMetrics.speed;
			}
		
			character.physicalBody.dynamicBody->ApplyForce(b2Vec2(force, 0), 
				character.physicalBody.dynamicBody->GetWorldCenter(), true);
		}

		bool hitsGround = 0;
		#pragma region jump
		{
			
			auto b = character.physicalBody.dynamicBody;

			for (b2ContactEdge *ce = b->GetContactList(); ce; ce = ce->next)
			{
				if (ce->contact->IsTouching())
				{
					if (
						ce->contact->GetFixtureA() == characterJumpSensor ||
						ce->contact->GetFixtureB() == characterJumpSensor
						)
					{
						hitsGround = true;
					}
				}
			}

			static bool duringJump = 0;
			static float jumpTimer = 0;
			
			if ((input.buttons[pika::Button::Space].pressed() ||
				input.anyController.buttons[pika::Controller::A].pressed())
				&& hitsGround
				)
			{
				duringJump = true;
				jumpTimer = inputMetrics.jumpTimer;

				float impulse = character.physicalBody.dynamicBody->GetMass() * inputMetrics.initialJumpImpulse;
				character.physicalBody.dynamicBody->ApplyLinearImpulse(b2Vec2(0, -impulse),
					character.physicalBody.dynamicBody->GetWorldCenter(), true);
			}

			if ((input.buttons[pika::Button::Space].held() ||
				input.anyController.buttons[pika::Controller::A].held()
				) 
					&& duringJump
				)
			{
				jumpTimer -= input.deltaTime;

				if (jumpTimer < 0) { jumpTimer = 0; duringJump = false; }
				else
				{
					auto b = character.physicalBody.dynamicBody;

					b->ApplyForce(b2Vec2(0, b->GetMass() * -inputMetrics.jump), b->GetWorldCenter(), true);
				}

				
			}
			else { duringJump = false; }
	
		}
		#pragma endregion


	#pragma endregion

		int32 velocityIterations = 8;
		int32 positionIterations = 3;
		world.Step(input.deltaTime, velocityIterations, positionIterations);

	#pragma region follow player
		if (followPlayer)
		{
			renderer.currentCamera.follow(character.physicalBody.getPosCenter(), 2 * input.deltaTime, 0.f, 0.f,
				windowState.frameBufferW, windowState.frameBufferH);
		}
	#pragma endregion


		floor.render(renderer, Colors_White);

		for (int i = 0; i < 10; i++)
		{
			if (blocks[i].dynamicBody->GetType() == b2_dynamicBody)
			{
				blocks[i].render(renderer, Colors_Orange);
			}
			else if (blocks[i].dynamicBody->GetType() == b2_kinematicBody)
			{
				blocks[i].render(renderer, {0.5,0.7,0.1,1});
			}
			else
			{
				blocks[i].render(renderer, Colors_White);
			}
		}

		renderer.renderRectangle(character.getRenderPos(), sprites, Colors_White, {}, 0, atlas.get(0, 0));

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
			float thickness = 0.01;

			if (currentBodySelected == b) 
			{
				color = Colors_Blue;
				thickness = 0.02;
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

							if (f->IsSensor())
							{
								renderer.renderLine(p1, p2, Colors_Green, thickness * 2);
							}
							else
							{
								renderer.renderLine(p1, p2, color, thickness * 2);
							}
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

				movement *= 0.01;

				if (currentBodySelected)
				{
					currentBodySelected->SetLinearVelocity({movement.x,movement.y});
				}
			}
			
		}


	#pragma endregion


		renderer.flush();

	#pragma region imgui
		{
			ImGui::Begin("Game Editor");

			ImGui::DragFloat2("Camera pos", &renderer.currentCamera.position[0], 0.001);
			ImGui::DragFloat("Camera zoom", &renderer.currentCamera.zoom, 0.5, 10, 1000);
		
			ImGui::Checkbox("Follow player", &followPlayer);

			ImGui::Separator();

			ImGui::InputFloat("Speed", &inputMetrics.speed);
			ImGui::InputFloat("Jump", &inputMetrics.jump);
			ImGui::InputFloat("Stop speed", &inputMetrics.stopSpeed);
			ImGui::InputFloat("Jump timer", &inputMetrics.jumpTimer);
			ImGui::InputFloat("Initial jump", &inputMetrics.initialJumpImpulse);

			ImGui::Separator();

			if (hitsGround)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, {0,1,0,1});
				ImGui::Text("Hits ground: Yes");
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Text, {1,0,0,1});
				ImGui::Text("Hits ground: No");
			}

			ImGui::PopStyleColor();


			if (currentBodySelected)
			{
				ImGui::NewLine();
				ImGui::Separator();
				ImGui::Text("World editor");
				ImGui::NewLine();


				int item = currentBodySelected->GetType();
				ImGui::Combo("Body type: ", &item, "static\0kinematic\0dynamic\0");

				currentBodySelected->SetType((b2BodyType)item);

				auto pos = currentBodySelected->GetPosition();

				auto angle = currentBodySelected->GetAngle();

				{
					auto f = currentBodySelected->GetFixtureList();
					
					if (f)
					{
						auto s = f->GetShape();

						if (s->GetType() == b2Shape::Type::e_polygon)
						{
							b2PolygonShape *poligon = dynamic_cast<b2PolygonShape *>(s);

							glm::vec2 size = {};

							if (poligon)
							{
								glm::vec2 min{999999,999999};
								glm::vec2 max{-999999,-999999};

								for (int i = 0; i < poligon->m_count; i++)
								{
									glm::vec2 p1 = {poligon->m_vertices[i].x,poligon->m_vertices[i].y};

									if (p1.x > max.x) { max.x = p1.x; }
									if (p1.y > max.y) { max.y = p1.y; }

									if (p1.x < min.x) { min.x = p1.x; }
									if (p1.y < min.y) { min.y = p1.y; }
								}

								size = max - min;
							}

							auto newSize = size;

							ImGui::DragFloat2("Size: ", &newSize.x, 0.1);

							if (newSize != size)
							{
								currentBodySelected->DestroyFixture(f);

								b2PolygonShape boxShape;
								boxShape.SetAsBox(newSize.x / 2, newSize.y / 2);

								b2FixtureDef boxFixtureDef;
								boxFixtureDef.shape = &boxShape;
								boxFixtureDef.density = 1;
								currentBodySelected->CreateFixture(&boxFixtureDef);
							}

						}
					}
				
				}

				ImGui::DragFloat2("Pos: ", &pos.x, 0.1);
				ImGui::SliderAngle("Angle", &angle);

				currentBodySelected->SetTransform(pos, angle);

			}

			ImGui::End();
		}

		requestedInfo.writeEntireFileBinary(PIKA_RESOURCES_PATH "hollowknight/inputMetrics.bin",
			&inputMetrics, sizeof(inputMetrics));

	#pragma endregion


		
		return true;
	}

	//optional
	void destruct()
	{
	}

};

