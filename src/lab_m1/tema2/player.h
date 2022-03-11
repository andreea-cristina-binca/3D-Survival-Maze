#pragma once

#include "utils/glm_utils.h"


class Player
{
public:
	glm::vec3 position;
	float speed, rotation;
	glm::vec3 scale;
	glm::vec3 actualSize;

	Player();
	~Player();
};
