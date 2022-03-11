#pragma once

#include "utils/glm_utils.h"
#include <vector>


class Labyrinth
{
public:
	int length, width;
	std::vector<std::vector<glm::vec4>> labyrinth;
	glm::vec3 cellScale;
	std::vector<glm::vec3> winCells;

	Labyrinth();
	Labyrinth(int length, int width);
	~Labyrinth();
};
