#include "bullet.h"


Bullet::Bullet()
{
	this->position = glm::vec3(0);
	this->speed = 0;
	this->lifetime = 0;
	this->direction = glm::vec3(0);
	this->scale = glm::vec3(0);
}


Bullet::~Bullet()
{
}
