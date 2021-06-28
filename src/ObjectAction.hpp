#pragma once

#include "Renderable.hpp"
#include "mesh.hpp"
#include "glm/gtx/transform.hpp"
#include <memory>

class ObjectAction : public Renderable {
public:
	float start_time;
	ObjectAction(float t);
	
	// "Initialize" for time-point.
	virtual void activate(float t, glm::mat4 obj_transform);
	virtual void render(float, glm::mat4);
};
