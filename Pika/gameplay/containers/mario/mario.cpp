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

void Player::checkCollisionBrute(glm::vec2 &pos, glm::vec2 lastPos, Block *map, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch)
{
	glm::vec2 delta = pos - lastPos;
	const float BLOCK_SIZE = 1.f;

	glm::vec2 &dimensions = position.size;

	glm::ivec2 mapSize = {100,100};

	if (
		(pos.y < -dimensions.y)
		|| (pos.x < -dimensions.x)
		|| (pos.y > mapSize.x * BLOCK_SIZE)
		|| (pos.x > mapSize.y * BLOCK_SIZE)
		)
	{
		return;
	}

	glm::vec2 newPos = performCollision(map, {pos.x, lastPos.y}, {dimensions.x, dimensions.y}, {delta.x, 0},
		upTouch, downTouch, leftTouch, rightTouch);
	pos = performCollision(map, {newPos.x, pos.y}, {dimensions.x, dimensions.y}, {0, delta.y},
		upTouch, downTouch, leftTouch, rightTouch);

}

glm::vec2 Player::performCollision(Block *map, glm::vec2 pos, glm::vec2 size, glm::vec2 delta, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch)
{
	glm::ivec2 mapSize = {100,100};

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

void Player::resolveConstrains(Block *map)
{
	glm::ivec2 mapSize = {100,100};

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


};
