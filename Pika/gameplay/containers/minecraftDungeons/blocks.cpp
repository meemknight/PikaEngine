#include "blocks.h"



int atlasData[] = 
{
	//front
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//back
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//top
	0, 0,
	0, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 4,//sand_stone
	2, 11,// snow_grass
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	4, 15, //block shelf
	5, 14, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//bottom
	0, 0,
	2, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	5, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 2,//sand_stone
	2, 15, // snow_grass
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	4, 15, //block shelf
	5, 14, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//left
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose

	//right
	0, 0,
	3, 15, //grass
	2, 15, // dirt
	1, 15, //stone
	3, 11, //ice
	4, 14,//log
	4, 15,//wooden_plank
	0, 14,//cobblestone
	7, 14,//gold_block
	7, 15,//bricks
	2, 14,//sand
	0, 3,//sand_stone
	4, 11,//snow_dirt
	5, 12,//leaves
	0, 13, // gold ore
	2, 13, // coal ore
	6, 12, //stone brick
	1, 13, // iron ore
	2, 12, // diamond ore
	3, 13, //block shelf
	5, 8, //birch wood
	3, 14, //gravel
	7, 13,//herbs
	12, 15,//rose



};

const int padd = BlockTypes::BlocksCount * 2;

glm::vec2 getAtlasTop(int type)
{
	return glm::vec2( atlasData[padd * 2 + type * 2] , atlasData[padd * 2 + type * 2 + 1] );
}

glm::vec2 getAtlasBottom(int type)
{
	return glm::vec2(atlasData[padd * 3 + type * 2], atlasData[padd * 3 + type * 2 + 1]);
}

glm::vec2 getAtlasLeft(int type)
{
	return glm::vec2(atlasData[padd * 4 + type * 2], atlasData[padd * 4 + type * 2 + 1]);
}

glm::vec2 getAtlasRight(int type)
{
	return glm::vec2(atlasData[padd * 5 + type * 2], atlasData[padd * 5 + type * 2 + 1]);
}

glm::vec2 getAtlasFront(int type)
{
	return glm::vec2(atlasData[type * 2], atlasData[type * 2 + 1]);
}

glm::vec2 getAtlasBack(int type)
{
	return glm::vec2(atlasData[padd + type * 2], atlasData[padd + type * 2 + 1]);
}