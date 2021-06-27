#include "ObjectAction.hpp"

ObjectAction::ObjectAction(float t) : start_time{t} {}
void ObjectAction::activate(float, Object &) {}
void ObjectAction::render(float, glm::mat4) {}
