

#include "mcDungeonsgameplay.h"



void  McDungeonsGameplay::resolveConstrains(PhysicsComponent &player)
{
	glm::ivec2 mapSize = {150,150};

	bool upTouch = 0;
	bool downTouch = 0;
	bool leftTouch = 0;
	bool rightTouch = 0;

	glm::vec2 &pos = player.position;

	float distance = glm::length(player.lastPos - pos);
	const float BLOCK_SIZE = 1.f;

	if (distance < BLOCK_SIZE)
	{
		checkCollisionBrute(player, 
			pos,
			player.lastPos,
			upTouch,
			downTouch,
			leftTouch,
			rightTouch
		);
	}
	else
	{
		glm::vec2 newPos = player.lastPos;
		glm::vec2 delta = pos - player.lastPos;
		delta = glm::normalize(delta);
		delta *= 0.9 * BLOCK_SIZE;

		do
		{
			newPos += delta;
			glm::vec2 posTest = newPos;
			checkCollisionBrute(player,
				newPos,
				player.lastPos,
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

		checkCollisionBrute(player,
			pos,
			player.lastPos,
			upTouch,
			downTouch,
			leftTouch,
			rightTouch);
	}

end:

	//clamp the box if needed
	//if (pos.x < 0) { pos.x = 0; leftTouch = true; }
	//if (pos.x + player.size.x > (mapSize.x) * BLOCK_SIZE)
	//{
	//	pos.x = ((mapSize.x) * BLOCK_SIZE) - player.size.x; rightTouch = true;
	//}

	return;

}

void  McDungeonsGameplay::checkCollisionBrute(PhysicsComponent &player, glm::vec2 &pos, glm::vec2 lastPos
	, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch)
{
	glm::vec2 delta = pos - lastPos;
	const float BLOCK_SIZE = 1.f;

	glm::vec2 &dimensions = player.size;

	glm::ivec2 mapSize = {150,150};

	if (
		(pos.y < -dimensions.y)
		|| (pos.x < -dimensions.x)
		|| (pos.y > mapSize.x * BLOCK_SIZE)
		|| (pos.x > mapSize.y * BLOCK_SIZE)
		)
	{
		return;
	}

	glm::vec2 newPos = performCollision(player, {pos.x, lastPos.y}, {dimensions.x, dimensions.y}, {delta.x, 0},
		upTouch, downTouch, leftTouch, rightTouch);
	pos = performCollision(player, {newPos.x, pos.y}, {dimensions.x, dimensions.y}, {0, delta.y},
		upTouch, downTouch, leftTouch, rightTouch);
}


const int playerPosY = 13;
glm::vec2  McDungeonsGameplay::performCollision(PhysicsComponent &player, glm::vec2 pos, glm::vec2 size,
	glm::vec2 delta, bool &upTouch, bool &downTouch, bool &leftTouch, bool &rightTouch)
{


	auto aabb = [](glm::vec4 b1, glm::vec4 b2, float delta)
	{
		b2.x += delta;
		b2.y += delta;
		b2.z -= delta * 2;
		b2.w -= delta * 2;

		if (((b1.x - b2.x < b2.z)
			&& b2.x - b1.x < b1.z
			)
			&& ((b1.y - b2.y < b2.w)
			&& b2.y - b1.y < b1.w
			)
			)
		{
			return 1;
		}
		return 0;
	};


	glm::ivec2 mapSize = {150,150};

	int minX = 0;
	int minY = 0;
	int maxX = mapSize.x;
	int maxY = mapSize.y;

	auto &dimensions = player.size;

	const float BLOCK_SIZE = 1.f;

	minX = (pos.x - abs(delta.x) - BLOCK_SIZE - size.x/2.f) / BLOCK_SIZE;
	maxX = ceil((pos.x + abs(delta.x) + BLOCK_SIZE + size.x / 2.f) / BLOCK_SIZE);

	minY = (pos.y - abs(delta.y) - BLOCK_SIZE - size.y / 2.f) / BLOCK_SIZE;
	maxY = ceil((pos.y + abs(delta.y) + BLOCK_SIZE + size.y/2.f) / BLOCK_SIZE);

	minX = std::max(0, minX);
	minY = std::max(0, minY);
	maxX = std::min(mapSize.x, maxX);
	maxY = std::min(mapSize.y, maxY);

	for (int y = minY; y < maxY; y++)
		for (int x = minX; x < maxX; x++)
		{
			if (
				getBlockUnsafe(x, playerPosY, y) != 0 ||
				getBlockUnsafe(x, playerPosY + 1, y) != 0 ||
				getBlockUnsafe(x, playerPosY-1, y) ==0
				)
			{
				if (aabb({pos - dimensions / 2.f,dimensions}, {x * BLOCK_SIZE - BLOCK_SIZE / 2.f, y * BLOCK_SIZE - BLOCK_SIZE / 2.f, BLOCK_SIZE, BLOCK_SIZE}, 0.0001f))
				{
					if (delta.x != 0)
					{
						if (delta.x < 0) // moving left
						{
							leftTouch = 1;
							pos.x = x * BLOCK_SIZE + BLOCK_SIZE/2.f + dimensions.x/2.f;
							goto end;
						}
						else
						{
							rightTouch = 1;
							pos.x = x * BLOCK_SIZE - BLOCK_SIZE / 2.f - dimensions.x / 2.f;
							goto end;
						}
					}
					else if (delta.y != 0)
					{
						if (delta.y < 0) //moving up
						{
							upTouch = 1;
							pos.y = y * BLOCK_SIZE + BLOCK_SIZE / 2.f + dimensions.y / 2.f;
							goto end;
						}
						else
						{
							downTouch = 1;
							pos.y = y * BLOCK_SIZE - BLOCK_SIZE / 2.f - dimensions.y/2.f;
							goto end;
						}
					}

				}
			}

		}

end:
	return pos;
}