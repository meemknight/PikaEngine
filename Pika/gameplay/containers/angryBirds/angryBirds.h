#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include "ph2d/ph2d.h"
#include <engineLibraresSupport/engineGL2DSupport.h>
#include <imguiComboSearch.h>

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
		pig,
	};

	glm::ivec2 sizes[9]{
		glm::ivec2{40,40},
		glm::ivec2{40,40},

		glm::ivec2(204, 20),
		glm::ivec2(204, 20),
		glm::ivec2(204, 20),

		glm::ivec2(80,80),
		glm::ivec2(80,80),
		glm::ivec2(80,80),

		glm::ivec2(70,70),

	};

	float friction[9]
	{
		0.8, 0.8, 0.8, 0.6, 0.8, 0.6, 0.8, 0.8, 0.8
	};

	float elasticity[9]
	{
		0.4, 0.4, 0.2, 0.2, 0.4, 0.2, 0.4, 0.2, 0.9
	};

	const char *textureNames[9]
	{
		PIKA_RESOURCES_PATH "sus/woodballs.png",
		PIKA_RESOURCES_PATH "sus/woodsmall.png",
		PIKA_RESOURCES_PATH "sus/stoneplank.png",
		PIKA_RESOURCES_PATH "sus/glassplank.png",
		PIKA_RESOURCES_PATH "sus/woodplank.png",
		PIKA_RESOURCES_PATH "sus/glass.png",
		PIKA_RESOURCES_PATH "sus/wood.png",
		PIKA_RESOURCES_PATH "sus/stone.png",
		PIKA_RESOURCES_PATH "sus/pig.png",
	};

	gl2d::Texture textures[9];
	gl2d::TextureAtlas texturesAtlas[9];
	gl2d::Texture background;
	gl2d::Texture sling;
	glm::vec2 backgroundSize;

	gl2d::Renderer2D renderer;
	ph2d::PhysicsEngine physicsEngine;
	ph2d::ph2dBodyId selectedID = 0;

	struct GameBlock
	{
		int textureType = 0;
		int materialType = 0;
	};

	std::unordered_map<ph2d::ph2dBodyId, GameBlock> gameBlocks;

	void addBlock(int type, glm::vec2 pos)
	{
		glm::ivec2 size = sizes[type];
		ph2d::ph2dBodyId body = 0;

		if (type == woodBalls || type == pig)
		{
			body = physicsEngine.addBody(pos,
				ph2d::createCircleCollider(size.x/2));

			GameBlock b;
			b.textureType = type;
			gameBlocks.emplace(body, b);
		}
		else
		{
			body = physicsEngine.addBody(pos,
				ph2d::createBoxCollider(size));

			GameBlock b;
			b.textureType = type;
			gameBlocks.emplace(body, b);
		}

		physicsEngine.bodies[body].dynamicFriction = friction[type];
		physicsEngine.bodies[body].staticFriction = friction[type] + 0.1f;
		physicsEngine.bodies[body].elasticity = elasticity[type];

		
	}

	void removeBlock(ph2d::ph2dBodyId id)
	{
		gameBlocks.erase(id);
		physicsEngine.bodies.erase(id);
	}

	//todo user can request imgui ids; shortcut manager context; allocators
	static ContainerStaticInfo containerInfo()
	{
		ContainerStaticInfo info = {};
		info.defaultHeapMemorySize = pika::MB(40);
		info.requestImguiFbo = true;
		info.pushAnImguiIdForMe = true;
		info.andInputWithWindowHasFocus = 1;
		info.andInputWithWindowHasFocusLastFrame = 1;

		return info;
	}

	bool create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
	{
		renderer.create(requestedInfo.requestedFBO.fbo);

		for (int i = 0; i < 9; i++)
		{
			textures[i] = pika::gl2d::loadTexture(textureNames[i], requestedInfo);

			if (i == pig)
			{
				texturesAtlas[i] = gl2d::TextureAtlas(1, 1);
			}
			else
			{
				texturesAtlas[i] = gl2d::TextureAtlas(3, 1);
			}
		}

		physicsEngine.simulationphysicsSettings.gravity = glm::vec2(0, 9.81) * 100.f;
		physicsEngine.simulationphysicsSettings.restingAngularVelocity = glm::radians(5.f);
		physicsEngine.simulationphysicsSettings.restingVelocity = 5;

		background = pika::gl2d::loadTexture(PIKA_RESOURCES_PATH "sus/background.png", requestedInfo);
		background.bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		backgroundSize = background.GetSize();

		sling = pika::gl2d::loadTexture(PIKA_RESOURCES_PATH "sus/sling.png", requestedInfo);

		for (int i = 0; i < 0; i++)
		{
			if (1)
			{
				float w = rand() % 100 + 20;
				float h = rand() % 100 + 20;
		
				auto body = physicsEngine.addBody({rand() % 800 + 100, rand() % 800 + 100},
					ph2d::createBoxCollider({w, h}));
				physicsEngine.bodies[body].motionState.rotation = ((rand() % 800) / 800.f) * 3.14159f;
			}
		
			for (int j = 0; j < 1; j++)
			{
				float r = rand() % 35 + 10;
		
				physicsEngine.addBody({rand() % 800 + 100, rand() % 800 + 100},
					ph2d::createCircleCollider({r}));
			}
		}

		//floor
		physicsEngine.addHalfSpaceStaticObject({0, 800}, {0.0, 1});

		return true;
	}

	std::vector<const char *>options = {
	"woodBalls",
	"woodSmall",
	"stoneplank",
	"glassplank",
	"woodplank",
	"glassblock",
	"woodblock",
	"stoneblock",
	"pig",
	"eraser",
	"move",
	"none",
	};

	bool radioButtonGroup(const char *label, const char *options[], int options_count, int &current_option)
	{
		bool selection_changed = false;

		ImGui::Text("%s", label); // Display the group label
		for (int i = 0; i < options_count; ++i)
		{
			if (ImGui::RadioButton(options[i], current_option == i))
			{
				current_option = i;
				selection_changed = true;
			}
			if (i < options_count - 1)
			{
				ImGui::SameLine(); // Place options horizontally
			}
		}

		return selection_changed;
	}

	int currentSelectedOption = 10;
	bool simulate = 1;
	bool snowGeometryOutlines = true;

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

	#pragma region imgui

		ImGui::Begin("Game window");

		ImGui::ComboWithFilter("Select object", &currentSelectedOption, options);
		//radioButtonGroup("Select object", options, sizeof(options)/sizeof(options[0]), currentSelectedOption);
	
		ImGui::Checkbox("Simulate physics", &simulate);
		ImGui::Checkbox("Show Geometry outlines", &snowGeometryOutlines);


		if (selectedID != 0)
		{
			auto found = physicsEngine.bodies.find(selectedID);

			if (found != physicsEngine.bodies.end())
			{
				ImGui::Separator();

				ImGui::Text("Object id: %d", selectedID);
				ImGui::DragFloat2("Position: ", &found->second.motionState.pos[0]);
				ImGui::SliderAngle("Angle: ", &found->second.motionState.rotation);
			}
			else
			{
				selectedID = 0;
			}


		}

		ImGui::End();

	#pragma endregion


	#pragma region update simulation

		if (simulate)
		{
			physicsEngine.runSimulation(input.deltaTime);
		}

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

		if (input.lMouse.pressed() && currentSelectedOption < 9)
		{
			addBlock(currentSelectedOption, mousePosWorld);

		}

		if (input.lMouse.pressed() && currentSelectedOption == 9)
		{

			for (auto &b : physicsEngine.bodies)
			{

				if (b.second.intersectPoint(mousePosWorld) &&
					b.second.collider.type != ph2d::ColliderHalfSpace
					)
				{
					removeBlock(b.first);
					break;
				}

			}
		}

		if (input.lMouse.pressed() && currentSelectedOption == 10)
		{
			for (auto &b : physicsEngine.bodies)
			{

				if (b.second.intersectPoint(mousePosWorld) &&
					b.second.collider.type != ph2d::ColliderHalfSpace
					)
				{
					selectedID = b.first;
				}

			}
		}

		//if (input.lMouse.pressed())
		//{
		//	addBlock(0, mousePosWorld);
		//}


	#pragma region render


		glm::vec4 backgroundPos{-backgroundSize, backgroundSize * 3.f};
		backgroundPos.y += 130;
		renderer.renderRectangle(backgroundPos,
			background, Colors_White, {}, 0, {-1,2,2,-1});

		renderer.renderRectangle({0, 720, 50, 80}, sling);

		for (auto &index : physicsEngine.bodies)
		{
			auto &b = index.second;

			auto itFound = gameBlocks.find(index.first);

			if (itFound != gameBlocks.end())
			{
				int textureIndex = itFound->second.textureType;

				renderer.renderRectangle(b.getAABB().asVec4(),
					textures[textureIndex],
					Colors_White,
					{}, glm::degrees(b.motionState.rotation), texturesAtlas[textureIndex].get(0, 0));

			}
		}

		if (currentSelectedOption < 9 && input.hasFocus && input.lastFrameHasFocus)
		{
			glm::vec4 aabb = glm::vec4(mousePosWorld, sizes[currentSelectedOption]);
			aabb.x -= aabb.z / 2;
			aabb.y -= aabb.w / 2;

			renderer.renderRectangle(aabb,
				textures[currentSelectedOption],
				{1,1,1,0.8},
				{}, 0, texturesAtlas[currentSelectedOption].get(0, 0));
		}

	#pragma endregion


	#pragma region render debug

		if(snowGeometryOutlines)
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

