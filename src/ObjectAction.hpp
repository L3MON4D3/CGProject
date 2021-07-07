#pragma once

#include "Renderable.hpp"
#include "mesh.hpp"
#include "glm/gtx/transform.hpp"
#include <memory>

class ObjectAction : public Renderable {
public:
	float from;
	float until;
	ObjectAction(float from, float until);
	
	// "Initialize" for time-point.
	virtual void activate(float t, glm::mat4 obj_transform);
	bool active(float t);
	virtual void render(float, glm::mat4);
};
