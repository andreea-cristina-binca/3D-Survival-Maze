#include "labyrinth.h"


Labyrinth::Labyrinth()
{
	this->length = 0;
	this->width = 0;
	this->cellScale = glm::vec3(0);
}

Labyrinth::Labyrinth(int length, int width)
{
	this->length = length;
	this->width = width;
	this->cellScale = glm::vec3(0);
}


Labyrinth::~Labyrinth()
{
}
