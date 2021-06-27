#include "LaserAction.hpp"

LaserAction::LaserAction(glm::mat4 model_transform) :
	model_transform{model_transform},
	active_transform{glm::identity<glm::mat4>()} { }
