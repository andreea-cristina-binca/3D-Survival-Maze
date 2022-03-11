#pragma once

#include "utils/glm_utils.h"


class Bullet
{
public:
	glm::vec3 position;
	float speed, lifetime;
	glm::vec3 direction;
	glm::vec3 scale;

	Bullet();
	~Bullet();
};
