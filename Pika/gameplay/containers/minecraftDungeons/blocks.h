#pragma once
#include <glm/vec2.hpp>

enum BlockTypes
{
	air = 0,
	grassBlock,
	dirt,
	stone,
	ice,
	woodLog,
	wooden_plank,
	cobblestone,
	gold_block,
	bricks,
	sand,
	sand_stone,
	snow_dirt,
	leaves,
	gold_ore,
	coar_ore,
	stoneBrick,
	iron_ore,
	diamond_ore,
	bookShelf,
	birch_wood,
	gravel,
	grass,
	rose,
	iron_block,
	glowStone,
	redstone_ore,
	crafting_table,
	furnace_on,
	furnace_off,
	BlocksCount
};


glm::vec2 getAtlasTop(int type);
glm::vec2 getAtlasBottom(int type);
glm::vec2 getAtlasLeft(int type);
glm::vec2 getAtlasRight(int type);
glm::vec2 getAtlasFront(int type);
glm::vec2 getAtlasBack(int type);
