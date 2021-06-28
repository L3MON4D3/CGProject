#include "ObjectAction.hpp"

ObjectAction::ObjectAction(float t) : start_time{t} {}
void ObjectAction::activate(float, glm::mat4) {}
void ObjectAction::render(float, glm::mat4) {}
