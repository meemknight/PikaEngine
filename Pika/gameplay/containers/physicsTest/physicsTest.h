#pragma once

#include <gl2d/gl2d.h>
#include <imgui.h>
#include <baseContainer.h>
#include <shortcutApi/shortcutApi.h>
#include <pikaSizes.h>
#include <imgui_spinner.h>
#include <ph2d/ph2d.h>


struct PhysicsTest: public Container
{

	gl2d::Renderer2D renderer;
	ph2d::PhysicsEngine physicsEngine;

	static constexpr float floorPos = 850;

	bool simulate = 1;

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
	
		for (int i = 0; i < 0; i++)
		{
			//if (i == 1) { mass = 0; }

			if (1)
			{
				float w = rand() % 100 + 20;
				float h = rand() % 100 + 20;

				physicsEngine.addBody({rand() % 800 + 100, rand() % 800 + 100},
					ph2d::createBoxCollider({w, h}));
				physicsEngine.bodies.back().motionState.rotation = ((rand() % 800) / 800.f) * 3.14159f;
			}

			for (int j = 0; j < 2; j++)
			{
				float r = rand() % 35 + 10;

				physicsEngine.addBody({rand() % 800 + 100, rand() % 800 + 100},
					ph2d::createCircleCollider({r}));
			}
		}


		glm::vec2 shape[5] = 
		{
			{0, -50},
			{40, -10},
			{25, 25},
			{-25, 25},
			{-40, -10},
		};
		physicsEngine.addBody({500, 200}, ph2d::createConvexPolygonCollider(shape, 5));


		//physicsEngine.addBody({500, 1100}, 
		//	ph2d::createBoxCollider({1100, 10}));

		//physicsEngine.addBody({500, 500}, ph2d::createBoxCollider({300, 100}));

		//physicsEngine.addBody({1, 800}, ph2d::createBoxCollider({800, 50}));
		//physicsEngine.bodies[1].motionState.mass = 0;
		//physicsEngine.bodies[1].motionState.momentOfInertia = 0;


		//physicsEngine.addBody({700, 700}, ph2d::createBoxCollider({300, 300}));


		//physicsEngine.addBody({600, 600}, ph2d::createBoxCollider({350, 350}));
		//physicsEngine.bodies[1].motionState.rotation = glm::radians(30.f);

		//physicsEngine.addBody({500, 500}, ph2d::createCircleCollider({75}));
		//physicsEngine.addBody({800, 100}, ph2d::createCircleCollider({55}));
		//physicsEngine.addBody({900, 500}, ph2d::createCircleCollider({40}));
		//physicsEngine.addBody({550, 700}, ph2d::createCircleCollider({25}));

		//physicsEngine.addBody({600, 600}, ph2d::createBoxCollider({50, 50}));


		//std::cout << ph2d::vectorToRotation({0,1}) << "\n";
		//std::cout << ph2d::vectorToRotation({-1,1}) << "\n";
		//std::cout << ph2d::vectorToRotation({-1,0}) << "\n";
		//std::cout << ph2d::vectorToRotation({0,-1}) << "\n";
		//std::cout << ph2d::vectorToRotation({1,0}) << "\n";
		//
		//std::cout << "\n";
		//std::cout << ph2d::rotationToVector(ph2d::vectorToRotation({0,1}) ).x << " " << ph2d::rotationToVector(ph2d::vectorToRotation({0,1}) ).y  << "\n";
		//std::cout << ph2d::rotationToVector(ph2d::vectorToRotation({-1,1})).x << " " << ph2d::rotationToVector(ph2d::vectorToRotation({-1,1})).y << "\n";
		//std::cout << ph2d::rotationToVector(ph2d::vectorToRotation({-1,0})).x << " " << ph2d::rotationToVector(ph2d::vectorToRotation({-1,0})).y << "\n";
		//std::cout << ph2d::rotationToVector(ph2d::vectorToRotation({0,-1})).x << " " << ph2d::rotationToVector(ph2d::vectorToRotation({0,-1})).y << "\n";
		//std::cout << ph2d::rotationToVector(ph2d::vectorToRotation({1,0}) ).x << " " << ph2d::rotationToVector(ph2d::vectorToRotation({1,0}) ).y  << "\n";

		physicsEngine.addHalfSpaceStaticObject({0, floorPos}, {0.0, 1});
		//physicsEngine.addBody({500, floorPos}, ph2d::createBoxCollider({900, 50}));
		//physicsEngine.bodies.back().motionState.mass = 0;
		//physicsEngine.bodies.back().motionState.momentOfInertia = 0;

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


		//ph2d::AABB box1 = glm::vec4{300,300, 300,300};
	//
	//ph2d::AABB box2 = glm::vec4(glm::vec2(platform::getRelMousePosition()) - glm::vec2(50, 50), 100, 100);
	//
	//renderer.renderRectangleOutline(box1.asVec4(), Colors_White);
	//
	//auto color = Colors_White;
	//
	//if (ph2d::AABBvsAABB(box1, box2))
	//{
	//	color = Colors_Red;
	//}
	//
	//renderer.renderRectangleOutline(box2.asVec4(), color);


	//ph2d::Circle a = glm::vec3{450,450, 150};
	//ph2d::Circle b = glm::vec3(glm::vec2(platform::getRelMousePosition()), 50);
	//renderer.renderCircleOutline(a.center, a.r, Colors_White, 2, 32);
	//
	//auto color = Colors_White;
	//if (ph2d::CirclevsCircle(a, b))
	//{
	//	color = Colors_Red;
	//}
	//renderer.renderCircleOutline(b.center, b.r, color, 2, 32);
		static int simulationSpeed = 1;
		float rightPos = 950;

		//physicsEngine.bodies[0].motionState.rotation = glm::radians(-30.f);


		ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.3,0.3,0.3,0.8});
		ImGui::Begin("Settings");

		ImGui::DragInt("Speed", &simulationSpeed);

		ImGui::SliderAngle("Angle", &physicsEngine.bodies[0].motionState.rotation);
		ImGui::SliderAngle("angular velocity", &physicsEngine.bodies[0].motionState.angularVelocity);
		//physicsEngine.bodies[0].motionState.angularVelocity = 1.5;

		ImGui::Text("Mouse pos %d, %d", input.mouseX, input.mouseY);

		ImGui::Text("Y min pos %f", physicsEngine.bodies[0].getAABB().min().y);
		ImGui::Text("Moment of inertia %f", physicsEngine.bodies[0].motionState.momentOfInertia);

		ImGui::Checkbox("Simulate", &simulate);

		ImGui::End();
		ImGui::PopStyleColor();

		static int selected = -1;

		//mouse
		if (!simulate && input.rMouse.pressed())
		{
			physicsEngine.bodies[0].motionState.setPos({input.mouseX, input.mouseY});
		}

		static glm::vec2 pressedPosition = {};

		if (input.lMouse.pressed())
		{
			selected = -1;

			for (int i = 0; i < physicsEngine.bodies.size(); i++)
			{
				if (physicsEngine.bodies[i].intersectPoint({input.mouseX, input.mouseY}))
				{
					selected = i;
					pressedPosition = {input.mouseX, input.mouseY};
				}
			}
		}

		if (selected >= 0)
		{
			renderer.renderLine(pressedPosition, {input.mouseX, input.mouseY}, Colors_Blue, 4);
		}

		if (input.lMouse.released() && selected >= 0)
		{

			glm::vec2 force = pressedPosition - glm::vec2({input.mouseX, input.mouseY});

			//physicsEngine.bodies[selected].motionState.velocity += force;
			force *= physicsEngine.bodies[selected].motionState.mass;
			force *= 2.f;

			physicsEngine.bodies[selected].motionState.applyImpulseWorldPosition(force,
				//physicsEngine.bodies[selected].motionState.pos
				pressedPosition
			);

			//physicsEngine.bodies[selected].motionState.angularVelocity = 10;

			selected = -1;
			pressedPosition = {};
		}


		for (int i = 0; i < simulationSpeed; i++)
		{

			//gravity
			if(simulate)
			for (int i=0; i<physicsEngine.bodies.size(); i++)
			{
				if(physicsEngine.bodies[i].motionState.mass != 0 && physicsEngine.bodies[i].motionState.mass != INFINITY)
					physicsEngine.bodies[i].motionState.acceleration += glm::vec2(0, 9.81) * 100.f;
			}

			if (simulate)
			{
				physicsEngine.collisionChecksCount = 8;
				physicsEngine.runSimulation(input.deltaTime);
			}
			else
			{
				physicsEngine.collisionChecksCount = 0;
				physicsEngine.runSimulation(0);
			}

			for (auto &b : physicsEngine.bodies)
			{
				auto bottom = b.getAABB().max().y;
				auto left = b.getAABB().min().x;
				auto right = b.getAABB().max().x;
				auto top = b.getAABB().min().y;

				//if (bottom > floorPos)
				//{
				//	float diff = bottom - floorPos;
				//	b.motionState.pos.y -= diff;
				//	b.motionState.lastPos = b.motionState.pos;
				//
				//	b.motionState.velocity.y *= -0.9;
				//}

				if (left < 0)
				{
					b.motionState.pos.x -= left;
					b.motionState.lastPos = b.motionState.pos;

					b.motionState.velocity.x *= -0.9;
				}

				if (right > rightPos)
				{
					b.motionState.pos.x -= right - rightPos;
					b.motionState.lastPos = b.motionState.pos;

					b.motionState.velocity.x *= -0.9;
				}

				if (top < 0)
				{
					b.motionState.pos.y -= top;
					b.motionState.lastPos = b.motionState.pos;

					b.motionState.velocity.y *= -0.9;
				}
			}
		}

		//std::cout << physicsEngine.bodies[0].getAABB().max().x << "\n";

		//renderer.renderRectangleOutline({300 - 25, 100 - 25, 50, 50}, Colors_Red);
		//renderer.renderRectangleOutline({600 - 25, 200 - 25, 50, 50}, Colors_Red);

		float p = 0;
		glm::vec2 n = {};
		bool penetrated = 0;
		glm::vec2 contactPoint = {};

		glm::vec2 tangentA = {};
		glm::vec2 tangentB = {};

		if (ph2d::BodyvsBody(physicsEngine.bodies[0],
			physicsEngine.bodies[1],
			p, n, contactPoint, tangentA, tangentB))
		{
			penetrated = true;
		}

		//if (ph2d::OBBvsOBB(physicsEngine.bodies[0].getAABB(),
		//	physicsEngine.bodies[0].motionState.rotation,
		//	physicsEngine.bodies[1].getAABB(),
		//	physicsEngine.bodies[1].motionState.rotation,
		//	p, n, contactPoint))
		//{
		//	penetrated = true;
		//}

		//auto a = physicsEngine.bodies[0].getAABB();
		//auto b = physicsEngine.bodies[1].getAABB();
		//ph2d::Circle a1 = glm::vec3{a.center(),a.size.x / 2};
		//ph2d::Circle b1 = glm::vec3{b.center(),b.size.x / 2};
		//if (ph2d::CirclevsCircle(a1, b1, p, n, contactPoint))
		//{
		//	penetrated = true;
		//}


		//auto a = physicsEngine.bodies[0].getAABB();
		//auto b = physicsEngine.bodies[1];
		//ph2d::LineEquation lineEquation;
		//lineEquation.createFromRotationAndPoint(b.motionState.rotation,
		//	b.motionState.pos);
		//if (ph2d::HalfSpaceVSCircle(lineEquation, a, p, n, contactPoint))
		//{
		//	penetrated = true;
		//}

		//glm::vec2 cornersA[4] = {};
		//glm::vec2 cornersB[4] = {};
		//physicsEngine.bodies[0].getAABB().getCornersRotated(cornersA, physicsEngine.bodies[0].motionState.rotation);
		//physicsEngine.bodies[1].getAABB().getCornersRotated(cornersB, physicsEngine.bodies[1].motionState.rotation);
		//
		//float rez = ph2d::calculatePenetrationAlongOneAxe(cornersA, 4, cornersB, 4, {1,0});
		//if (rez > 0)
		//{
		//	penetrated = true;
		//}

		ImGui::Begin("Settings");
		ImGui::Text("Penetration: %f", p);
		ImGui::End();

		for (int i = 0; i < physicsEngine.bodies.size(); i++)
		{
			auto &b = physicsEngine.bodies[i];

			auto color = Colors_White;

			if (i == selected)
			{
				color = Colors_Blue;
			}

			if (b.intersectPoint({input.mouseX, input.mouseY}))
			{
				color = Colors_Turqoise;
			}

			if (penetrated)
			{
				color = Colors_Red;
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

		//renderer.renderRectangle({-100, floorPos, 100000, 20});

		if (penetrated)
		{
			renderer.renderLine(contactPoint,
				contactPoint + n * 100.f, Colors_Green, 4);

			renderer.renderRectangle({contactPoint - glm::vec2(2,2), 4,4}, Colors_White);

			renderer.renderLine(contactPoint,
				contactPoint + tangentA * 100.f, Colors_Red, 4);


			renderer.renderLine(contactPoint,
				contactPoint + tangentB * 100.f, Colors_Purple, 4);
		}


		//glm::vec2 lineEquationStart = lineEquation.getClosestPointToOrigin();
		//lineEquationStart -= lineEquation.getLineVector() * 1000.f;
		//renderer.renderLine(lineEquationStart, lineEquationStart + lineEquation.getLineVector() * 2000.f, Colors_Red);


		//ph2d::LineEquation lineEquation;
		//lineEquation.createFromNormalAndPoint({0,1}, {0, floorPos});

		//float pl = 0;
		//pl = lineEquation.computeEquation(platform::getRelMousePosition());
		//ImGui::Begin("Settings");
		//ImGui::Text("Penetration line: %f", pl);
		//ImGui::End();


		//glm::vec2 p2 = platform::getRelMousePosition();
		//p2 = ph2d::rotateAroundCenter(p2, glm::radians(45.f));
		//renderer.renderRectangle({p2, 10, 10}, Colors_Red);




		renderer.flush();

		return true;
	}

	//optional
	void destruct(RequestedContainerInfo &requestedInfo)
	{

	}

};
