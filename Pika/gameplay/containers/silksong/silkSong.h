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
#include <safeSave/safeSave.h>
#include <engineLibraresSupport/engineSafeSaveSupport.h>
#include "imguiComboSearch.h"


struct SilkSong: public Container
{

	gl2d::Renderer2D renderer;
	gl2d::Texture sprites;
	gl2d::TextureAtlasPadding atlas;
	gl2d::FrameBuffer fbo;

	b2World world{{0, 10}};

	constexpr static int ASSETS_COUNT = 27;

	std::vector<const char*> assetsNames=
	{
		"bush.png",
		"grass.png",
		"mushroom1.png",
		"mushroom2.png",
		"mushroom3.png",
		"mushroom4.png",
		"mushroom5.png",
		"mushroom6.png",
		"vines1.png",
		"vines2.png",
		"vines3.png",
		"background.jpg",
		"grass2.png",
		"grass3.png",
		"block1.png",
		"block2.png",
		"block3.png",
		"block4.png",

		"bush2.png",
		"bush3.png",
		"grassDecoration.png",
		"grassDecorationBig.png",
		"mushroom7.png",
		"mushroom8.png",
		"stones.png",
		"stones2.png",
		"background2.jpg",


	};

	gl2d::Texture assets[ASSETS_COUNT];

	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(100);

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

		void create(b2World &world, glm::vec4 dimensions, float rotation, int type)
		{
			b2BodyDef myBodyDef;
			myBodyDef.type = (b2BodyType)type; //this will be a dynamic body

			dimensions.x += dimensions.z / 2.f;
			dimensions.y += dimensions.w / 2.f;

			myBodyDef.position.Set(dimensions.x, dimensions.y); //set the starting position
			myBodyDef.angle = rotation; //set the starting angle

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

	struct SavedBlock
	{
		glm::vec4 dimensions = {};
		float rotation = 0;
		int type = 0;
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

	//Block blocks[10];
	b2Body *currentBodySelected = 0;
	int currentSelectedSprite = 0;

	Entity character;
	b2Fixture *characterJumpSensor;

	glm::vec2 draggedStart = {};

	bool followPlayer = false;
	bool dragGeometry = false;
	bool renderGeometry = true;
	int selectType = 0;

	struct InputMetrict
	{
		float speed = 20;
		float jump = 8;
		float stopSpeed = 7;
		float jumpTimer = 1;
		float initialJumpImpulse = 4;
	} inputMetrics = {};

	struct GameSprite
	{
		glm::vec3 pos = {};
		float scale = 1;
		int type = 0;
		float rotation = 0;
		bool flip = 0;
		int layer = 0; //0 is background than 1 is objects than 2 is decoration than player than 3
		glm::vec4 color = {1,1,1,1};
	};

	std::vector<GameSprite> gameSprites;

	gl2d::ShaderProgram blurShader;

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{

		fbo.create(1, 1, true, true);

		renderer.create(requestedInfo.requestedFBO.fbo);
		renderer.currentCamera.zoom = 80.f;
		renderer.currentCamera.position.x = -440;
		renderer.currentCamera.position.y = -500;
		renderer.currentCamera3D.position.z = 2;


		blurShader = gl2d::createPostProcessShaderFromFile(PIKA_RESOURCES_PATH "hollowknight/blur.frag");


		if (!requestedInfo.readEntireFileBinary(PIKA_RESOURCES_PATH "hollowknight/inputMetrics.bin",
			&inputMetrics, sizeof(inputMetrics)))
		{
			inputMetrics = {};
		}

		for (int i = 0; i < ASSETS_COUNT; i++)
		{
			assets[i]
				= pika::gl2d::loadTexture
				((std::string(PIKA_RESOURCES_PATH "hollowknight/") + std::string(assetsNames[i])).c_str()
				, requestedInfo);
		}



		sprites = pika::gl2d::loadTextureWithPixelPadding(PIKA_RESOURCES_PATH "hollowknight/sprites.png", requestedInfo, 80);
		atlas = gl2d::TextureAtlasPadding(12, 12, sprites.GetSize().x, sprites.GetSize().y);

		world.SetAllowSleeping(true);
		world.SetContinuousPhysics(true);

		{
			sfs::SafeSafeKeyValueData data;

			pika::sfs::safeLoad(data, PIKA_RESOURCES_PATH "hollowknight/map2", false);

			void *geometry = 0;
			size_t s = 0;
			if (data.getRawDataPointer("geometry", geometry, s) == sfs::noError)
			{
				SavedBlock *b = (SavedBlock*)geometry;

				//for (int i = 0; i < s / sizeof(SavedBlock); i++)
				//{
				//	blocks[i].create(world, b[i].dimensions, b[i].rotation, b[i].type);
				//}
			}
			else
			{
				requestedInfo.log("Error reading map file", pika::logError);
			}

			void *loadedSprites = 0;
			s = 0;
			if (data.getRawDataPointer("sprites", loadedSprites, s) == sfs::noError)
			{
				GameSprite *loagedGameSprites = (GameSprite *)loadedSprites;

				for (int i = 0; i < s / sizeof(GameSprite); i++)
				{
					gameSprites.push_back(loagedGameSprites[i]);
				}
			}

		}

	
		character.physicalBody.create(world, {10, 2, 0.6f,1}, 0, b2BodyType::b2_dynamicBody);
		character.spriteDimensions = glm::vec4{0,0,1,1};
		character.physicalBody.dynamicBody->SetFixedRotation(true);
		characterJumpSensor = character.physicalBody.addSensor({0, 0.5, 0.55f, 0.1f});


		Block floor;
		floor.create(world, {-1000, 10, 2000, 1}, 0, b2BodyType::b2_staticBody);


		return true;
	}

	bool movingLeft = 0;

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
	#pragma region clear stuff
		//glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);
		fbo.resize(windowState.windowW, windowState.windowH);
	#pragma endregion

	#pragma region input
		requestedInfo.requestedFBO.depthTexture;

		//::pika::gl2d::cameraController(renderer.currentCamera, input, 60 * (!followPlayer), 4);

		{
			auto &c = renderer.currentCamera3D;
			c.use = true;
			float speed = 1;
			glm::vec3 position = {};
			if (input.buttons[pika::Button::W].held())
			{
				position.z -= speed * input.deltaTime;
			}
			if (input.buttons[pika::Button::S].held())
			{
				position.z += speed * input.deltaTime;
			}
			if (input.buttons[pika::Button::A].held())
			{
				position.x -= speed * input.deltaTime;
			}
			if (input.buttons[pika::Button::D].held())
			{
				position.x += speed * input.deltaTime;
			}	
			if (input.buttons[pika::Button::E].held())
			{
				position.y += speed * input.deltaTime;
			}
			if (input.buttons[pika::Button::Q].held())
			{
				position.y -= speed * input.deltaTime;
			}
			c.moveFPS(position);

			float rotateSpeed = 0.002;
			if (!input.rMouse.held()) { rotateSpeed = 0; }
			c.rotateFPS({input.mouseX, input.mouseY}, rotateSpeed);


		}

		int direction = 0;
		

		if (input.buttons[pika::Button::Left].held() 
			|| input.anyController.buttons[pika::Controller::Left].held() 
			|| input.anyController.LStick.left()
			) { direction--; movingLeft = true; }

		if (input.buttons[pika::Button::Right].held() 
			|| input.anyController.buttons[pika::Controller::Right].held()
			|| input.anyController.LStick.right()
			) { direction++; movingLeft = false; }

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

	#pragma region render

		//background

		{
			int index = 0;
			for (auto &s : gameSprites)
			{
				if (s.layer == 0)
				{
		
					if (currentSelectedSprite == index && selectType == 2)
					{
						renderer.renderRectangle({glm::vec2(s.pos), glm::vec2(assets[s.type].GetSize()) * s.scale * (1.f / 400.f)},
							assets[s.type], {1,0.7,0.7,1.0}, {}, s.rotation, s.flip ? glm::vec4(1, 1, 0, 0) : glm::vec4(0, 1, 1, 0), s.pos.z);
					}
					else
					{
						renderer.renderRectangle({glm::vec2(s.pos), glm::vec2(assets[s.type].GetSize()) * s.scale * (1.f / 400.f)},
							assets[s.type], s.color, {}, s.rotation, s.flip ? glm::vec4(1, 1, 0, 0) : glm::vec4(0, 1, 1, 0), s.pos.z);
					}
				}
		
				index++;
			}
		}
		renderer.flushPostProcess({blurShader});


		for (int i = 1; i < 4; i++)
		{
			//player
			if (i == 3)
			{
				renderer.renderRectangle(character.getRenderPos(), sprites, Colors_White, {}, 0, 
					atlas.get(0, 0, movingLeft));
			}

			int index = 0;
			for (auto &s : gameSprites)
			{
				if (s.layer == i)
				{

					if (currentSelectedSprite == index && selectType == 2)
					{
						renderer.renderRectangle({glm::vec2(s.pos), glm::vec2(assets[s.type].GetSize()) * s.scale * (1.f / 400.f)},
							assets[s.type], {1,0.7,0.7,1.0}, {}, s.rotation, s.flip ? glm::vec4(1, 1, 0, 0) : glm::vec4(0, 1, 1, 0), s.pos.z);
					}
					else
					{
						renderer.renderRectangle({glm::vec2(s.pos), glm::vec2(assets[s.type].GetSize()) * s.scale * (1.f / 400.f)},
							assets[s.type], s.color, {}, s.rotation, s.flip ? glm::vec4(1, 1, 0, 0) : glm::vec4(0, 1, 1, 0), s.pos.z);
					}
				}

				index++;
			}
		}


	#pragma endregion


		glm::vec2 mouseWorldpos(input.mouseX, input.mouseY);
		{
			auto viewRect = renderer.getViewRect();

			glm::vec2 mousePosNormalized = mouseWorldpos / glm::vec2(windowState.frameBufferW, windowState.frameBufferH);
			
			mouseWorldpos = glm::vec2(viewRect) + mousePosNormalized * glm::vec2(viewRect.z, viewRect.w);
		}

	#pragma region body render

		if(renderGeometry)
		for (b2Body *b = world.GetBodyList(); b; b = b->GetNext())
		{

			glm::vec4 color = Colors_Red;
			float thickness = 0.01;

			if (currentBodySelected == b && selectType == 1)
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

				if (shape->TestPoint(shapePos, {mouseWorldpos.x,mouseWorldpos.y}) && input.lMouse.held()
					&& selectType == 1
					)
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

		if(dragGeometry)
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

			//if (ImGui::Button("Spawn"))
			//{
			//	for (int i = 0; i < 10; i++)
			//	{
			//		blocks[i].create(world, {1 + sin(i) * 5, -1.5 * i, 1,1}, 0, b2BodyType::b2_dynamicBody);
			//	}
			//}

			ImGui::DragFloat2("Camera pos", &renderer.currentCamera.position[0], 0.001);
			ImGui::DragFloat("Camera zoom", &renderer.currentCamera.zoom, 0.5, 10, 1000);
		
			ImGui::Checkbox("Follow player", &followPlayer);
			ImGui::Checkbox("Drag geometry", &dragGeometry);
			ImGui::Checkbox("Render geometry", &renderGeometry);

			ImGui::Combo("Editor type: ", &selectType, "none\0geometry\0sprites\0");

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

			if(selectType == 1)
			if (currentBodySelected)
			{
				ImGui::NewLine();
				ImGui::Separator();
				ImGui::Text("Geometry editor");
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

				ImGui::DragFloat3("Pos: ", &pos.x, 0.1);
				ImGui::SliderAngle("Angle", &angle);

				currentBodySelected->SetTransform(pos, angle);

			}

			if (selectType == 2)
			{
				ImGui::NewLine();
				ImGui::Separator();
				ImGui::Text("Sprites editor");
				ImGui::NewLine();

				if (ImGui::Button("Add sprite"))
				{
					gameSprites.push_back({});
					currentSelectedSprite = gameSprites.size() - 1;
				}

				if (currentSelectedSprite > 0 && currentSelectedSprite < gameSprites.size())
				if (ImGui::Button("Remove sprite"))
				{
					gameSprites.erase(gameSprites.begin() + currentSelectedSprite);
				}
				
				ImGui::InputInt("Sprite", &currentSelectedSprite);

				currentSelectedSprite = glm::clamp(currentSelectedSprite, 0, (int)gameSprites.size() - 1);

				if (gameSprites.size())
				{

					auto &s = gameSprites[currentSelectedSprite];

					ImGui::ComboWithFilter("Type", &s.type, assetsNames);
					s.type = glm::clamp(s.type, 0, ASSETS_COUNT);

					ImGui::Combo("Layer: ", &s.layer, "background\0geometry\0geometry2\0on top of player\0");

					ImGui::DragFloat3("Pos", &s.pos[0], 0.01);

					ImGui::ColorEdit4("Color", &s.color[0]);

					ImGui::Checkbox("Flip", &s.flip);

					ImGui::SliderFloat("Angle", &s.rotation, -360, 360, "%.00f deg");

					ImGui::DragFloat("Scale", &s.scale, 0.01, 0.001, 100);


				}

			}


			ImGui::NewLine();
			ImGui::Separator();

			if (ImGui::Button("Save map"))
			{
				//SavedBlock b[10];
				//for (int i = 0; i < 10; i++)
				//{
				//	b[i].dimensions = glm::vec4(blocks[i].getPosTopLeft(), blocks[i].getSize());
				//	b[i].rotation = blocks[i].getRotation();
				//	b[i].type = blocks[i].dynamicBody->GetType();
				//}

				sfs::SafeSafeKeyValueData data;

				//data.setRawData("geometry", b, sizeof(b));

				data.setRawData("sprites", gameSprites.data(), sizeof(GameSprite) *gameSprites.size());


				pika::memory::pushCustomAllocatorsToStandard();
				sfs::safeSave(data, PIKA_RESOURCES_PATH "hollowknight/map2", false);
				pika::memory::popCustomAllocatorsToStandard();

			}


			ImGui::End();
		}

		//renderer.renderRectangle({0,0, renderer.windowW, renderer.windowH}, fbo.texture);
		//renderer.flushFBO(gl2d::FrameBuffer{requestedInfo.requestedFBO.fbo});

		//requestedInfo.writeEntireFileBinary(PIKA_RESOURCES_PATH "hollowknight/inputMetrics.bin",
		//	&inputMetrics, sizeof(inputMetrics));

	#pragma endregion


		
		return true;
	}

	//optional
	void destruct(RequestedContainerInfo &requestedInfo)
	{
	}

};

