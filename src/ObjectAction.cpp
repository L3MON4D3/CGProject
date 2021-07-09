#include "ObjectAction.hpp"

ObjectAction::ObjectAction(float from, float until) : from{from}, until{until} {}
void ObjectAction::activate(float, glm::mat4) {}

bool ObjectAction::active(float t) {
	return t >= from && t <= until;
}

void ObjectAction::render(float, glm::mat4, glm::mat4) {}
