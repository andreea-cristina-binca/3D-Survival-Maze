#pragma once

#include "utils/glm_utils.h"


class Enemy
{
public:
	glm::vec3 position;
	float speed;
	glm::vec3 scale;

	Enemy(glm::vec3 position, float speed, glm::vec3 scale);
	~Enemy();
};
