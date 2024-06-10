#include "mario.h"


namespace mario
{


glm::vec2 Transform::getTopLeftCorner()
{
	return position;
}

glm::vec2 Transform::getCenter()
{
	return position + size / 2.f;
}

glm::vec2 Transform::getBottomLeftCorner()
{
	return position + glm::vec2(0, size.y);
}

glm::vec2 Transform::getBottomCenter()
{
	return position + glm::vec2(size.x / 2.f, size.y);
}



void Player::move(glm::vec2 dir)
{
	if (length(dir) == 0.f) { return; }

	movingThisFrame = true;
	position.position += dir;
}


float maxMoveVelocity = 25;
float startVelocity = 10;

void Player::moveVelocityX(float dir)
{
	if (dir == 0) { return; }

	movingThisFrame = true;

	if (dir > 0 && velocity.x < 0)
	{
		velocity.x = 0;
	}
	else if (dir < 0 && velocity.x > 0)
	{
		velocity.x = 0;
	}

	if (dir > 0)
	{
		if (velocity.x < maxMoveVelocity)
		{
			if (velocity.x == 0)
			{
				velocity.x = startVelocity;
			}

			velocity.x += dir;

			if (velocity.x > maxMoveVelocity)
			{
				velocity.x = maxMoveVelocity;
			}
		}
	}
	else if (dir < 0)
	{
		if (velocity.x > -maxMoveVelocity)
		{
			if (velocity.x == 0)
			{
				velocity.x = -startVelocity;
			}

			velocity.x += dir;

			if (velocity.x < -maxMoveVelocity)
			{
				velocity.x = -maxMoveVelocity;
			}
		}
	}



}

void Player::jump(float power)
{
	if (grounded)
	{
		velocity.y = -power;
	}
}

const float terminalVelocity = 60;

void Player::applyGravity(float gravity)
{

	if (velocity.y < terminalVelocity)
	{
		velocity.y += gravity;

		if (velocity.y > terminalVelocity)
		{
			velocity.y = terminalVelocity;
		}
	}

}


void Player::updateMove()
{
	if (lastPos.x - position.position.x < 0)
	{
		movingRight = true;
	}
	else if (lastPos.x - position.position.x > 0)
	{
		movingRight = false;
	}

	if (movingThisFrame)
	{
		//playerAnimation.state = PlayerAnimation::STATES::running;
	}
	else
	{
		//playerAnimation.state = {};
	}


	lastPos = position.position;

	movingThisFrame = false;

}

float groundDrag = 120.f;
float airDrag = 60.f;

void Player::updatePhisics(float deltaTime)
{

	glm::vec2 drag = {groundDrag, groundDrag};

	if (!grounded) { drag.x = airDrag; }

	if (!movingThisFrame)
	{
		if (velocity.x > 0)
		{
			velocity -= drag * deltaTime;

			if (velocity.x < 0)
			{
				velocity.x = 0;
			}
		}
		else if (velocity.x < 0)
		{
			velocity += drag * deltaTime;

			if (velocity.x > 0)
			{
				velocity.x = 0;
			}
		}
	}

	position.position += velocity * deltaTime;

}

void Player::checkCollisionBrute(glm::vec2 &pos, glm::vec2 lastPos, Block *map, glm::ivec2 mapSize, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch)
{
	glm::vec2 delta = pos - lastPos;
	const float BLOCK_SIZE = 1.f;

	glm::vec2 &dimensions = position.size;

	if (
		(pos.y < -dimensions.y)
		|| (pos.x < -dimensions.x)
		|| (pos.y > mapSize.y * BLOCK_SIZE)
		|| (pos.x > mapSize.x * BLOCK_SIZE)
		)
	{
		return;
	}

	glm::vec2 newPos = performCollision(map, mapSize, {pos.x, lastPos.y}, {dimensions.x, dimensions.y}, {delta.x, 0},
		upTouch, downTouch, leftTouch, rightTouch);
	pos = performCollision(map, mapSize, {newPos.x, pos.y}, {dimensions.x, dimensions.y}, {0, delta.y},
		upTouch, downTouch, leftTouch, rightTouch);

}

glm::vec2 Player::performCollision(Block *map, glm::ivec2 mapSize, glm::vec2 pos, glm::vec2 size, glm::vec2 delta, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch)
{
	int minX = 0;
	int minY = 0;
	int maxX = mapSize.x;
	int maxY = mapSize.y;

	auto &dimensions = position.size;

	const float BLOCK_SIZE = 1.f;

	minX = (pos.x - abs(delta.x) - BLOCK_SIZE) / BLOCK_SIZE;
	maxX = ceil((pos.x + abs(delta.x) + BLOCK_SIZE + size.x) / BLOCK_SIZE);

	minY = (pos.y - abs(delta.y) - BLOCK_SIZE) / BLOCK_SIZE;
	maxY = ceil((pos.y + abs(delta.y) + BLOCK_SIZE + size.y) / BLOCK_SIZE);

	minX = std::max(0, minX);
	minY = std::max(0, minY);
	maxX = std::min(mapSize.x, maxX);
	maxY = std::min(mapSize.y, maxY);

	auto getMapBlockUnsafe = [&](int x, int y) -> Block *
	{
		return &map[x + y * mapSize.x];
	};

	for (int y = minY; y < maxY; y++)
		for (int x = minX; x < maxX; x++)
		{
			if (getMapBlockUnsafe(x, y)->isCollidable())
			{
				if (aabb({pos,dimensions}, {x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE}, 0.0001f))
				{
					if (delta.x != 0)
					{
						if (delta.x < 0) // moving left
						{
							leftTouch = 1;
							pos.x = x * BLOCK_SIZE + BLOCK_SIZE;
							goto end;
						}
						else
						{
							rightTouch = 1;
							pos.x = x * BLOCK_SIZE - dimensions.x;
							goto end;
						}
					}
					else if (delta.y != 0)
					{
						if (delta.y < 0) //moving up
						{
							upTouch = 1;
							pos.y = y * BLOCK_SIZE + BLOCK_SIZE;
							goto end;
						}
						else
						{
							downTouch = 1;
							pos.y = y * BLOCK_SIZE - dimensions.y;
							goto end;
						}
					}

				}
			}

		}

end:
	return pos;

}

void Player::resolveConstrains(Block *map, glm::ivec2 mapSize)
{
	bool upTouch = 0;
	bool downTouch = 0;
	bool leftTouch = 0;
	bool rightTouch = 0;

	glm::vec2 &pos = position.position;

	float distance = glm::length(lastPos - pos);
	const float BLOCK_SIZE = 1.f;

	if (distance < BLOCK_SIZE)
	{
		checkCollisionBrute(pos,
			lastPos,
			map,
			mapSize,
			upTouch,
			downTouch,
			leftTouch,
			rightTouch
		);
	}
	else
	{
		glm::vec2 newPos = lastPos;
		glm::vec2 delta = pos - lastPos;
		delta = glm::normalize(delta);
		delta *= 0.9 * BLOCK_SIZE;

		do
		{
			newPos += delta;
			glm::vec2 posTest = newPos;
			checkCollisionBrute(newPos,
				lastPos,
				map,
				mapSize,
				upTouch,
				downTouch,
				leftTouch,
				rightTouch);

			if (newPos != posTest)
			{
				pos = newPos;
				goto end;
			}

		} while (glm::length((newPos + delta) - pos) > 1.0f * BLOCK_SIZE);

		checkCollisionBrute(pos,
			lastPos,
			map,
			mapSize,
			upTouch,
			downTouch,
			leftTouch,
			rightTouch);
	}

end:

	//clamp the box if needed
	if (pos.x < 0) { pos.x = 0; leftTouch = true; }
	if (pos.x + position.size.x > (mapSize.x) * BLOCK_SIZE)
	{
		pos.x = ((mapSize.x) * BLOCK_SIZE) - position.size.x; rightTouch = true;
	}


	if (leftTouch && velocity.x < 0) { velocity.x = 0; }
	if (rightTouch && velocity.x > 0) { velocity.x = 0; }

	if (upTouch && velocity.y < 0) { velocity.y = 0; }
	if (downTouch && velocity.y > 0) { velocity.y = 0; }

	if (downTouch) { grounded = true; }

}

void getVision(char vision[visionSizeX * visionSizeY], mario::GameplaySimulation &simulator, PlayerSimulation &p)
{
	memset(vision, 0, sizeof(vision));
	for (int y = 0; y < visionSizeY; y++)
	{
		for (int x = 0; x < visionSizeX; x++)
		{
			auto b = simulator.
				getMapBlockSafe(
				x + p.p.position.getCenter().x - 1,
				y + p.p.position.getCenter().y - visionSizeY + 10);

			vision[x + y * visionSizeX] = b.isCollidable();
		}
	}
}

bool performNeuralSimulation(PlayerSimulation &p, float deltaTime, mario::GameplaySimulation &simulator, mario::NeuralNetork
	&network)
{
	if (p.p.position.position.x - p.maxPosition > 0.2)
	{

		p.maxPosition = p.p.position.position.x;
		p.killTimer = 0;
	}
	else
	{
		p.killTimer += deltaTime;
		if (p.killTimer > 4)
		{
				return 0;
		}
	}

	p.maxFit = p.maxPosition - p.jumpCount * 5;

	//simulator.moveDelta = 1;
	//simulator.jump = 0;
	char vision[visionSizeX * visionSizeY];
	getVision(vision, simulator, p);

	//input
	network.compute(simulator.moveDelta, simulator.jump, vision);

	if (simulator.jump && p.p.grounded)
	{
		p.jumpCount++;
		//p.maxFit -= 1;
	}

	if (!simulator.updateFrame(deltaTime, p.p))
	{
		return 0;
	}
	else
	{
		return 1;
	}

}

void renderNeuralNetwork(gl2d::Renderer2D &renderer, char vision[mario::visionSizeX * mario::visionSizeY], float blockSizePreview, mario::NeuralNetork &network)
{
	auto renderLine = [&](glm::vec2 a, glm::vec2 b, glm::vec4 color)
	{
		float dist = glm::distance(a, b);
		float thickness = 5;

		glm::vec2 vect = b - a;

		float angle = -std::atan2(vect.y, vect.x);
		//float angle = 0;

		renderer.renderRectangle({a, dist, thickness}, color, {-dist / 2.f,0}, glm::degrees(angle));
	};

	renderer.pushCamera();
	renderer.renderRectangle(
		glm::vec4(0, 0, mario::visionSizeX * (float)blockSizePreview, mario::visionSizeY * (float)blockSizePreview)
		, {0.5,0.5,0.5,0.5});
	for (int y = 0; y < mario::visionSizeY; y++)
	{
		for (int x = 0; x < mario::visionSizeX; x++)
		{
			auto b = vision[x + y * mario::visionSizeX];
			if (b == 1)
			{
				renderer.renderRectangle(glm::vec4(x, y, 0.95, 0.95) * (float)blockSizePreview, {0,1,0,0.5});
			}
			//else
			//{
			//	renderer.renderer.renderRectangle(glm::vec4(x, y, 0.95, 0.95) * (float)blockSizePreview, {0.5,0.5,0.5,0.5});
			//}
		}
	}

	renderer.renderRectangle(
		glm::vec4(
		1,
		mario::visionSizeY - 10,
		PLAYER_SIZE) *
		(float)blockSizePreview, {0,0,1,0.5});

	glm::vec2 upkeyPositions(mario::visionSizeX * blockSizePreview * 4,
		mario::visionSizeY * blockSizePreview * 0.5 - blockSizePreview * 2);

	glm::vec2 leftkeyPositions(mario::visionSizeX * blockSizePreview * 4,
		mario::visionSizeY * blockSizePreview * 0.5);

	glm::vec2 rightkeyPositions(mario::visionSizeX * blockSizePreview * 4,
		mario::visionSizeY * blockSizePreview * 0.5 + blockSizePreview * 2);

	glm::vec2 firstNeuronPositions(mario::visionSizeX * blockSizePreview * 1.5,
		mario::visionSizeY * blockSizePreview * 0.5 - blockSizePreview * 5);

	glm::vec2 secondNeuronPositions(mario::visionSizeX * blockSizePreview * 2.5,
		mario::visionSizeY * blockSizePreview * 0.5 - blockSizePreview * 5);
	

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < mario::subLayerSize; j++)
		{
	
			if (network.weights3[i][j] != 0)
			{
				glm::vec4 color;
				if (network.weights3[i][j] > 0)
				{
					color = glm::vec4(0, 1, 0, network.weights3[i][j] / 1.5);
				}
				else
				{
					color = glm::vec4(1, 0, 0, network.weights3[i][j] / -1.5);
				}
	
				color.w = glm::clamp(color.w, 0.1f, 1.f);
	
				glm::vec2 positions[] = {upkeyPositions, leftkeyPositions, rightkeyPositions};
	
				renderLine(
					secondNeuronPositions + glm::vec2{0, 10 * j * blockSizePreview / mario::subLayerSize},
					positions[i] + glm::vec2(blockSizePreview) / 2.f,
					color);
	
			}
		}
	}

	for (int i = 0; i < mario::subLayerSize; i++)
	{
		for (int j = 0; j < mario::subLayerSize; j++)
		{

			if (network.weights2[i][j] != 0)
			{
				glm::vec4 color;
				if (network.weights2[i][j] > 0)
				{
					color = glm::vec4(0, 1, 0, network.weights2[i][j] / 1.5);
				}
				else
				{
					color = glm::vec4(1, 0, 0, network.weights2[i][j] / -1.5);
				}

				color.w = glm::clamp(color.w, 0.1f, 1.f);


				renderLine(
					firstNeuronPositions + glm::vec2{0, 10 * j * blockSizePreview / mario::subLayerSize},
					secondNeuronPositions + glm::vec2{0, 10 * i * blockSizePreview / mario::subLayerSize},
					color);

			}
		}
	}

	for (int i = 0; i < mario::subLayerSize; i++)
	{
		for (int j = 0; j < mario::visionTotal; j++)
		{

			if (network.weights1[i][j] != 0)
			{
				glm::vec4 color;
				if (network.weights1[i][j] > 0)
				{
					color = glm::vec4(0, 1, 0, network.weights1[i][j] / 1.5);
				}
				else
				{
					color = glm::vec4(1, 0, 0, network.weights1[i][j] / -1.5);
				}

				color.w = glm::clamp(color.w, 0.1f, 1.f);


				renderLine(
					(glm::vec2(j % mario::visionSizeX, j / mario::visionSizeX) + glm::vec2(0.5, 0.5)) * (float)blockSizePreview,
					firstNeuronPositions + glm::vec2{0, 10 * i * blockSizePreview / mario::subLayerSize},
					color);

			}
		}
	}



	for (int i = 0; i < mario::subLayerSize; i++)
	{
		renderer.renderRectangle(
			glm::vec4(firstNeuronPositions + glm::vec2{0, 10 * i * blockSizePreview/ mario::subLayerSize}, 
			blockSizePreview/2, blockSizePreview/2)
			, {1,0,1,0.5});
	}


	for (int i = 0; i < mario::subLayerSize; i++)
	{
		renderer.renderRectangle(
			glm::vec4(secondNeuronPositions + glm::vec2{0, 10 * i * blockSizePreview / mario::subLayerSize},
			blockSizePreview / 2, blockSizePreview / 2)
			, {1,0,1,0.5});
	}



	renderer.renderRectangle(
		glm::vec4(upkeyPositions, blockSizePreview, blockSizePreview)
		, {0,0,1,0.5});
	renderer.renderRectangle(
		glm::vec4(leftkeyPositions, blockSizePreview, blockSizePreview)
		, {0,0,1,0.5});
	renderer.renderRectangle(
		glm::vec4(rightkeyPositions, blockSizePreview, blockSizePreview)
		, {0,0,1,0.5});

	renderer.popCamera();
}

};
