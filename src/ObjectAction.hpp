#pragma once

#include "Renderable.hpp"
#include "Object.hpp"

// Forward-declare Object.
class Object;

class ObjectAction : public Renderable {
public:
	float start_time;
	ObjectAction(float t);
	// "Initialize" with o.
	
	virtual void activate(float t, Object &o);
	virtual void render(float, glm::mat4);
};
