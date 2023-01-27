#pragma once
#include <gl2d/gl2d.h>



constexpr const char *collisionMap =
"XX-X-XXX"
"XX-XXXXX"
"--------"
"-XX--X--"
"-XX-----"
"-XX-XXXX"
"--X-XXX-"
"XX------"
"XX--XX--"
"XXXXXX--"
;

bool isSolid(int id);

glm::vec4 getTileUV(gl2d::TextureAtlasPadding atlas, int id, int flip = 0);

struct Block
{
	unsigned char type : 7;
	unsigned char flipped : 1;

	bool isCollidable()
	{
		return collisionMap[type] == 'X';
	}
};



bool aabb(glm::vec4 b1, glm::vec4 b2, float delta);