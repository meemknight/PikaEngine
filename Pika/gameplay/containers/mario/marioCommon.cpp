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
