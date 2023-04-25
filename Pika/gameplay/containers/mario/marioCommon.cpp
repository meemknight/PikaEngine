#include "marioCommon.h"

namespace mario
{


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

bool loadMap(RequestedContainerInfo &requestedInfo, std::string file, Block **map, glm::ivec2 &mapSize)
{
	
	delete[] * map;

	if (!requestedInfo.readEntireFileBinary(file, &mapSize, sizeof(mapSize)))
	{
		return 0;
	}

	*map = new Block[mapSize.x * mapSize.y];
	Block d{27,0};
	memset(*map, *(int *)(&d), mapSize.x * mapSize.y);

	size_t s = 0;
	if (requestedInfo.getFileSizeBinary(file.c_str(), s))
	{
		if (s == mapSize.x * mapSize.y + sizeof(mapSize))
		{
			requestedInfo.readEntireFileBinary(file.c_str(), *map, mapSize.x * mapSize.y, sizeof(mapSize));
		}
		else 
		{
			requestedInfo.consoleWindow->write("Error: mario file is corrupt\n");
			return 0; 
		}
	}
	else { return 0; }

	return 1;
}



};

