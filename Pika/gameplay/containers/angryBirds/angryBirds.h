#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include "ph2d/ph2d.h"
#include <engineLibraresSupport/engineGL2DSupport.h>


struct AngryBirds: public Container
{

	enum blockTypes
	{
		woodBalls,
		woodSmall,
		stoneplank,
		glassplank,
		woodplank,
		glassblock,
		woodblock,
		stoneblock,
	};

	glm::ivec2 sizes[8]{
		glm::ivec2{40,40},
		glm::ivec2{40,40},

		glm::ivec2(204, 20),
		glm::ivec2(204, 20),
		glm::ivec2(204, 20),

		glm::ivec2(80,80),
		glm::ivec2(80,80),
		glm::ivec2(80,80),
	};

	const char *textureNames[8]
	{
		PIKA_RESOURCES_PATH "sus/woodballs.png",
		PIKA_RESOURCES_PATH "sus/woodsmall.png",
		PIKA_RESOURCES_PATH "sus/stoneplank.png",
		PIKA_RESOURCES_PATH "sus/glassplank.png",
		PIKA_RESOURCES_PATH "sus/woodplank.png",
		PIKA_RESOURCES_PATH "sus/glass.png",
		PIKA_RESOURCES_PATH "sus/wood.png",
		PIKA_RESOURCES_PATH "sus/stone.png",
	};

	gl2d::Texture textures[8];
	gl2d::TextureAtlas texturesAtlas[8];

	gl2d::Renderer2D renderer;
	ph2d::PhysicsEngine physicsEngine;

	struct GameBlock
	{
		int textureType = 0;
		int materialType = 0;
	};

	std::unordered_map<ph2d::ph2dBodyId, GameBlock> gameBlocks;

	void addBlock(int type, glm::vec2 pos)
	{
		glm::ivec2 size = sizes[type];

		if (type == woodBalls)
		{
			auto body = physicsEngine.addBody(pos,
				ph2d::createCircleCollider(size.x/2));

			GameBlock b;
			b.textureType = type;
			gameBlocks.emplace(body, b);
		}
		else
		{
			auto body = physicsEngine.addBody(pos,
				ph2d::createBoxCollider(size));

			GameBlock b;
			b.textureType = type;
			gameBlocks.emplace(body, b);
		}

	}

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(10);
		info.requestImguiFbo = true;
		info.pushAnImguiIdForMe = true;
		info.andInputWithWindowHasFocus = 0;
		info.andInputWithWindowHasFocusLastFrame = 0;

		return info;
	}


	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		renderer.create(requestedInfo.requestedFBO.fbo);

		for (int i = 0; i < 8; i++)
		{
			textures[i] = pika::gl2d::loadTexture(textureNames[i], requestedInfo);
			texturesAtlas[i] = gl2d::TextureAtlas(3, 1);
		}

		physicsEngine.simulationphysicsSettings.gravity = glm::vec2(0, 9.81) * 100.f;


		//for (int i = 0; i < 10; i++)
		//{
		//	if (1)
		//	{
		//		float w = rand() % 100 + 20;
		//		float h = rand() % 100 + 20;
		//
		//		auto body = physicsEngine.addBody({rand() % 800 + 100, rand() % 800 + 100},
		//			ph2d::createBoxCollider({w, h}));
		//		physicsEngine.bodies[body].motionState.rotation = ((rand() % 800) / 800.f) * 3.14159f;
		//	}
		//
		//	for (int j = 0; j < 1; j++)
		//	{
		//		float r = rand() % 35 + 10;
		//
		//		physicsEngine.addBody({rand() % 800 + 100, rand() % 800 + 100},
		//			ph2d::createCircleCollider({r}));
		//	}
		//}

		//floor
		physicsEngine.addHalfSpaceStaticObject({0, 800}, {0.0, 1});

		return true;
	}

	bool update(pika::Input input, pika::WindowState windowState,
		RequestedContainerInfo &requestedInfo)
	{
	#pragma region init stuff
		int w = 0; int h = 0;
		w = windowState.frameBufferW; //window w
		h = windowState.frameBufferH; //window h
		glClear(GL_COLOR_BUFFER_BIT); //clear screen
		renderer.updateWindowMetrics(w, h);
	#pragma endregion

	#pragma region update simulation

		physicsEngine.runSimulation(input.deltaTime);

	#pragma endregion

		pika::gl2d::cameraController(renderer.currentCamera, input, 100, 1);


		glm::vec2 mousePosScreen = {input.mouseX, input.mouseY};
		glm::vec2 mousePosWorld = {0, 0};

	#pragma region get cursor position
		auto viewRect = renderer.getViewRect();

		auto lerp = [](auto a, auto b, auto c)
		{
			return a * (1.f - c) + b * c;
		};

		mousePosWorld = lerp(glm::vec2(viewRect.x, viewRect.y),
			glm::vec2(viewRect.x + viewRect.z, viewRect.y + viewRect.w),
			glm::vec2(input.mouseX, input.mouseY) / glm::vec2(windowState.windowW, windowState.windowH));
	#pragma endregion

		if (input.lMouse.pressed())
		{
			addBlock(1, mousePosWorld);
		}


	#pragma region render

		for (auto &index : physicsEngine.bodies)
		{
			auto &b = index.second;

			auto itFound = gameBlocks.find(index.first);

			if (itFound != gameBlocks.end())
			{
				int textureIndex = itFound->second.textureType;

				//renderer.renderRectangle(b.getAABB().asVec4(),
				//	textures[textureIndex],
				//	Colors_White,
				//	{}, b.motionState.rotation, texturesAtlas[textureIndex].get(0, 0));

			}

			
		}

	#pragma endregion


	#pragma region render debug

		for (auto &index : physicsEngine.bodies)
		{
			auto &b = index.second;

			auto color = Colors_White;

			if (b.intersectPoint(mousePosWorld))
			{
				color = Colors_Turqoise;
			}
		
			//if (OBBvsPoint(physicsEngine.bodies[i].getAABB(),
			//	physicsEngine.bodies[i].motionState.rotation,
			//	{input.mouseX, input.mouseY}))
			//{
			//	color = Colors_Blue;
			//}

			if (b.collider.type == ph2d::ColliderCircle)
			{
				renderer.renderCircleOutline(b.motionState.pos,
					b.collider.collider.circle.radius, color, 2, 32);

				glm::vec2 vector = {1,0};
				vector = ph2d::rotateAroundCenter(vector, b.motionState.rotation);
				renderer.renderLine(b.motionState.pos, b.motionState.pos + vector *
					b.collider.collider.circle.radius, color, 4);

			}
			else if (b.collider.type == ph2d::ColliderBox)
			{
				float rotation = glm::degrees(b.motionState.rotation);

				renderer.renderRectangleOutline(b.getAABB().asVec4(), color, 2, {}, rotation);
			}
			else if (b.collider.type == ph2d::ColliderHalfSpace)
			{

				ph2d::LineEquation lineEquation;
				lineEquation.createFromRotationAndPoint(b.motionState.rotation,
					b.motionState.pos);

				glm::vec2 lineEquationStart = lineEquation.getClosestPointToOrigin();
				lineEquationStart -= lineEquation.getLineVector() * 1000.f;
				renderer.renderLine(lineEquationStart, lineEquationStart + lineEquation.getLineVector() * 2000.f, Colors_Red);
			}
			else if (b.collider.type == ph2d::ColliderConvexPolygon)
			{
				auto &c = b.collider.collider.convexPolygon;

				for (int i = 0; i < c.vertexCount; i++)
				{
					glm::vec2 p1 = c.vertexesObjectSpace[i] + b.motionState.pos;
					glm::vec2 p2 = c.vertexesObjectSpace[(i + 1) % c.vertexCount] + b.motionState.pos;

					p1 = ph2d::rotateAroundPoint(p1, b.motionState.pos, b.motionState.rotation);
					p2 = ph2d::rotateAroundPoint(p2, b.motionState.pos, b.motionState.rotation);

					renderer.renderLine(p1, p2, color);
				}

			}

		}

		renderer.renderRectangle({mousePosWorld - glm::vec2(3,3), 6, 6}, Colors_Red);

	#pragma endregion




		renderer.flush();


		return true;
	}

	//optional
	void destruct(RequestedContainerInfo &requestedInfo)
	{

	}

};

