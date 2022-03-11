#include "enemy.h"

Enemy::Enemy(glm::vec3 position, float speed, glm::vec3 scale)
{
	this->position = position;
	this->speed = speed;
	this->scale = scale;
}

Enemy::~Enemy()
{
}
