#include "player.h"


Player::Player()
{
	position = glm::vec3(0);
	speed = 0;
	rotation = 0;
	scale = glm::vec3(0);
	actualSize = glm::vec3(0);
}


Player::~Player()
{
}
