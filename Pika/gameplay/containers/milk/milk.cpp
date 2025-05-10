#include "milk.h"


float vertexDataOriginal[] = {
	//front
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, 0.5, 0.5,

	//back
	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,

	//top
	-0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,

	//bottom
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,

	//left
	-0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, 0.5,

	//right
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,
};

float vertexUVOriginal[] = {
	//front
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//back
	0, 0,
	0, 1,
	1, 1,
	1, 1,
	1, 0,
	0, 0,

	//top
	0, 0,
	0, 1,
	1, 1,
	1, 1,
	1, 0,
	0, 0,

	//bottom
	1, 1,
	0, 1,
	0, 0,
	0, 0,
	1, 0,
	1, 1,

	//left
	1, 0,
	1, 1,
	0, 1,
	0, 1,
	0, 0,
	1, 0,

	//right
	0, 1,
	1, 1,
	1, 0,
	1, 0,
	0, 0,
	0, 1,
};

gl3d::Model Milk::createCubeModel(glm::vec3 size)
{

	std::vector<float> data;
	data.reserve(2000);
	for (int face = 0; face < 6; face++)
	{


		glm::vec3 normal = {};
		glm::vec3 positions[3] = {};

		for (int i = 0; i < 3; i++)
		{
			positions[i].x = vertexDataOriginal[i * 3 + face * 3 * 6 + 0];
			positions[i].y = vertexDataOriginal[i * 3 + face * 3 * 6 + 1];
			positions[i].z = vertexDataOriginal[i * 3 + face * 3 * 6 + 2];
		}

		glm::vec3 edge1 = positions[1] - positions[0];
		glm::vec3 edge2 = positions[2] - positions[0];
		normal = glm::normalize(glm::cross(edge1, edge2));

		for (int i = 0; i < 6; i++)
		{

			data.push_back(vertexDataOriginal[i * 3 + face * 3 * 6 + 0] * size.x);
			data.push_back(vertexDataOriginal[i * 3 + face * 3 * 6 + 1] * size.y);
			data.push_back(vertexDataOriginal[i * 3 + face * 3 * 6 + 2] * size.z);

			data.push_back(normal.x);
			data.push_back(normal.y);
			data.push_back(normal.z);

			if (face == 0)
			{
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 0] * size.x * 0.25);
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 1] * size.y * 0.25);
			}
			else if (face == 1)
			{
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 0] * size.x * 0.25);
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 1] * size.y * 0.25);
			}
			else if (face == 2)
			{
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 0] * size.x * 0.25);
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 1] * size.z * 0.25);
			}
			else if (face == 3)
			{
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 0] * size.x * 0.25);
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 1] * size.z * 0.25);
			}
			else if (face == 4)
			{
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 0] * size.z * 0.25);
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 1] * size.y * 0.25);
			}
			else if (face == 5)
			{
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 0] * size.z * 0.25);
				data.push_back(vertexUVOriginal[i * 2 + face * 2 * 6 + 1] * size.y * 0.25);
			}


		}
	}

	/*
	position					vec3
	normals						vec3
	(optional) texcoords		vec2
	(optional) joints id		ivec4
	(optional) joints weights	vec4
	*/

	return renderer.createModelFromData(floorMaterial, "cube", data.size(), data.data());
	
}
