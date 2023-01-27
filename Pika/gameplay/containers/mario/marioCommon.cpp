#include "marioCommon.h"



bool isSolid(int id)
{
	return(collisionMap[id] == 'X');
}

glm::vec4 getTileUV(gl2d::TextureAtlasPadding atlas, int id, int flip)
{
	int x = id % 8;
	int y = id / 8;
	return atlas.get(x, y, flip);
}

bool aabb(glm::vec4 b1, glm::vec4 b2, float delta)
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
}
